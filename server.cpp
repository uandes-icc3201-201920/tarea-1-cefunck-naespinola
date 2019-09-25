#include <iostream>
#include <memory>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ctime>
#include <math.h>
#include "util.h"
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>


using namespace std;
struct ClientThreadData {
	//pthread_rwlock_t lock;
	bool read_mode;
	int rc, cl;
	char buf[100];
};

// Almacenamiento KV
KVStore db;
unsigned long global_key_counter;


int server_socket_file_descriptor(int listen_len_queue, char* socket_path){
	struct sockaddr_un addr;
	int fd;

	//Se crea un socket de tipo SCK_STREAM y protocolo 0
	if ( (fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("socket error");
		return -1;
  	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
		strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);
	unlink(socket_path);

    //Establecer vinculacion con la dirección local del socket
	if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		perror("bind error");
		return -1;
	}

	//Escuchar conexion de cliente
	if (listen(fd, listen_len_queue) == -1) { //permite una cola de "listen_len_queue" conexiones
		perror("listen error");
		return -1;
	}
	cout << "socket listening..." << endl;
	return fd;
}


Value string_to_value(string str_data){
	vector<byte> data(str_data.begin(),str_data.end());
	Value value = { str_data.size(), data};
	return value;
}


string value_to_string(Value value){
	string str_data(value.data.begin(), value.data.end());
	return str_data;
}


bool is_valid_key(unsigned long key){
	if( key < 1 || (floor(key) != key))
		return false;
	return true;
}


vector<unsigned long> list(){
	vector<unsigned long> keys;
	for(map<unsigned long, Value>::iterator it = db.begin(); it != db.end(); ++it) {
		keys.push_back(it->first);
	}
	return keys;
}


bool peek(unsigned long key){
	map<unsigned long,Value>::iterator it;
	it = db.find(key);
  if (it != db.end())
    return true;
  return false;
}


int delete_from_db(unsigned long key){
	if (!peek(key))
		return key;
	db.erase(key);
	return 0;
}


bool update_into_db(unsigned long key, Value value){
	if(!peek(key))
		return false;
	db[key] = value;
	return true;
}


Value get(unsigned long key){
	if(peek(key))
		return db[key];
	Value error = string_to_value("Error: no existe la clave ingresada");
	return error;
}


unsigned long random_key(){
	srand((unsigned)time(0));
	unsigned long i;
	i = (rand()%10000)+1000;
	return i;
}


void init_db(){
	global_key_counter = random_key();
}


int insert_key_value_into_db(unsigned long key, Value value){
	if (peek(key) || !is_valid_key(key)){
		return -1;
	}
	db.insert(pair<unsigned long, Value>(key, value));
	return key;
}


int insert_value_into_db(Value value){
	pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
	unsigned long new_key = global_key_counter;
	int result = insert_key_value_into_db(new_key, value);
	if (result == new_key){
		pthread_mutex_lock(&lock);
		global_key_counter++; //seccion critica
		pthread_mutex_unlock(&lock);
	}
	return result;
}

vector<string> split(string str, string token){
    vector<string>result;
    while(str.size()){
        int index = str.find(token);
        if(index!=string::npos){
            result.push_back(str.substr(0,index));
            str = str.substr(index+token.size());
            if(str.size()==0)result.push_back(str);
        }else{
            result.push_back(str);
            str = "";
        }
    }
    return result;
}

string exec_statement(string statement){
	string response = "0 ";
	unsigned long key;
	Value value;
	int integer_result;
	string str_result;
	vector<string> splitted_statement = split(statement," ");
	switch (stoi(splitted_statement[1])) {
		case 1:// connect
			return response + "cliente ya está conectado";
			break;
		case 2:// disconnect
			return response + "el cliente debe cerrar su socket";
			break;
		case 3:// quit
			return response + "el cliente debe cerrar su socket y matar el proceso";
			break;
		case 4:// insert
			if(splitted_statement.size() == 3){
				//std::cout << "value:" << splitted_statement[2] << '\n';
				value = string_to_value(splitted_statement[2]);
				integer_result = insert_value_into_db(value);
				if(integer_result != -1){
					response = response + to_string(integer_result);
				}else{
					response = response + "Error: clave invalida";
				}
			}else if(splitted_statement.size() == 4){
				//std::cout << "key:"<<splitted_statement[2] << '\n';
				//std::cout << "value:"<<splitted_statement[3] << '\n';
				key = stoi(splitted_statement[2]);
				value = string_to_value(splitted_statement[3]);
				int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock);
				integer_result = insert_key_value_into_db(key,value); //seccion critica
				int pthread_rwlock_unlock(pthread_rwlock_t *rwlock);
				if(integer_result != -1){
					response = response + to_string(integer_result);
				}else{
					response = response + "Error: clave invalida";
				}
			}
			return response;
			break;
		case 5:// get
			key = stoi(splitted_statement[2]);
			value = get(key);
			response = response + value_to_string(value);
			return response;
			break;
		case 6:// peek
			key = stoi(splitted_statement[2]);
			if(peek(key)){
				response = response + "true";
			}else{
				response = response + "false";
			}
			return response;
			break;
		case 7:// update
			key = stoi(splitted_statement[2]);
			value = string_to_value(splitted_statement[3]);
			if(update_into_db(key,value)){
				response = response + "actualización exitosa";
			}else{
				response = response + "Error: clave invalida para actualización";
			}
			return response;
			break;
		case 8:// delete
			key = stoi(splitted_statement[2]);
			if(delete_from_db(key) == 0){
				response = response + "Eliminación exitosa";
			}
			else{
				response = response + to_string(key);
			}
			return response;
			break;
		case 9:// list
			vector<unsigned long> keys = list();
			for(vector<unsigned long>::iterator it = keys.begin(); it != keys.end(); ++it) {
				str_result += to_string(*it) + " ";
			}
			return response + str_result;
			break;
	}
}

void * command_processing(void * client_thread_data){
	struct ClientThreadData *ct_data = (struct ClientThreadData *)client_thread_data;
	bool read_mode = ct_data->read_mode;
	char buf[100];
	strncpy(buf, ct_data->buf,sizeof(ct_data->buf));
	int rc = ct_data->rc;
	int cl = ct_data->cl;
	string client_output = "";
	string server_output = "";
	while (1){
		while(!read_mode){
			cout << "Modo escritura server" << endl;
			//std::cout << "para el cliente:" << cl << endl;
			if(client_output != ""){
				server_output = exec_statement(client_output);
				rc = server_output.length();
				const void * temp_buf = server_output.c_str();
				if (write(cl, temp_buf, rc) != rc){
					if (rc > 0){
						fprintf(stderr,"partial write");
					} else {
						perror("write error");exit(-1);
					}
				}
				if(*buf=='0'){
					read_mode = true;
					//client_output = "";
					break;
				}
			}
		}

		while(read_mode){
			cout << "Modo lectura server" << endl;
			//std::cout << "para el cliente:" << cl << endl;
			if ( (rc=read(cl,buf,sizeof(buf))) > 0) {
				//printf("read %u bytes: %.*s\n", rc, rc, buf);
				string temp(buf);
				client_output = temp.substr(0,rc);
				//std::cout << "client_output" << client_output << '\n';
			}
			if (rc == -1) {
				perror("read");
				exit(-1);
			}
			else if (rc == 0) {
				printf("EOF\n");
				read_mode = false;
				//close(cl);
			}
			if(*buf=='0'){
				read_mode = false;
			}
		}
	}
}

int main(int argc, char** argv) {
	init_db();
	vector<pthread_t> thread_ids;
	char *socket_path = "/tmp/db.tuples.sock";
  int fd,rc,opt;

	// Procesar opciones de linea de comando
	while ((opt = getopt (argc, argv, "s:")) != -1) {
        switch (opt)
		{
			/* Procesar el flag s si el usuario lo ingresa */
			case 's':
				socket_path = optarg;
				break;
			default:
				return EXIT_FAILURE;
          }
    }
  fd = server_socket_file_descriptor(99, socket_path);
	while(1){
		char buf[100];
		int cl = accept(fd, NULL, NULL);
		if(cl == -1){
			perror("accept error");
		}else{
			bool read_mode = true;
			struct ClientThreadData *ct_data;
			ct_data = (struct ClientThreadData *)malloc(sizeof(struct ClientThreadData));
			ct_data->read_mode = read_mode;
			//ct_data->buf = buf;
			strncpy(ct_data->buf, buf, sizeof(buf));
			ct_data->rc = rc;
			ct_data->cl = cl;
			pthread_t new_client_thread;
			pthread_create(&new_client_thread,NULL,command_processing,(void *)ct_data);
			thread_ids.push_back(new_client_thread);
		}
	}

	return 0;
}
