#include <iostream>
#include <memory>
#include <string>
#include <unistd.h>
#include <algorithm>
#include <math.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "util.h"

using namespace std;
struct ConnectionData {
	bool time_out, successful_connection;
	char * socket_path;
};



int client_socket_file_descriptor(char *socket_path){
	int fd;
	struct sockaddr_un addr;

	if ( (fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("socket error");
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);

	if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
	  	perror("connect error");
	  	return -1;
	}
	return fd;
}

void * socket_connect(void * flags){
	int fd = -1;
	struct ConnectionData *f = (struct ConnectionData *)flags;
	while(!(f->time_out) && !(f->successful_connection)){
		fd = client_socket_file_descriptor(f->socket_path);
		if(fd != -1){
			f->successful_connection = true;
			cout << "conexión exitosa" << endl;
		}
		sleep(1);
	}
	return (void *)fd;
}

void * timer( void * flags){
	struct ConnectionData *f = (struct ConnectionData *)flags;
	int seconds = 0;
	while ((seconds < 10) && !(f->successful_connection)) {
		seconds++;
		sleep(1);
	}
	f-> time_out = true;
	return NULL;
}

int connection_attempt(char *socket_path){
	void * result;
	int fd = -1;
	struct ConnectionData *flags;
	flags = (struct ConnectionData *)malloc(sizeof(struct ConnectionData));
	flags->time_out = false;
	flags->successful_connection = false;
	flags->socket_path = socket_path;
	printf("%s\n", socket_path);
	pthread_t timer_thread;
	pthread_t connector_thread;
	pthread_create(&timer_thread, NULL, timer, (void *)flags);
	pthread_create(&connector_thread, NULL, socket_connect, (void *)flags);
	pthread_join(connector_thread, &result);
	fd = *((int*)(&result));
	free(flags);
	if(fd == -1){
		cout << "error: el tiempo máximo para conectar al servidor expiró" << endl;
	}
	return fd;
}

string to_lower(string str){
	transform(str.begin(), str.end(),str.begin(), ::tolower);
	return str;
}

bool is_valid_key(string str_key){
	unsigned long key;
	if(str_key.front() != '-'){
		try{
			key = stoi(str_key);
			if( key > 0 && (floor(key) == key))
				return true;
			}catch(const std::invalid_argument& ia){
				return false;
			}
			return false;
		}
	}

bool validate_statement(vector<string> splitted_statement){
	vector<string> commands_without_args{"connect","disconnect","quit","list"};
	vector<string> commands_with_1_arg{"connect","insert","get","peek","delete"};
	vector<string> commands_with_2_arg{"insert","update"};
	string command;
	string key;
	if(splitted_statement.size() < 1){
		return false;
	}
	command = splitted_statement[0];
	if(splitted_statement.size() > 1){
		if(splitted_statement.size() == 2){
			if(splitted_statement[1].back() != ')') {return false;}
			else{
			key = splitted_statement[1].substr(0,splitted_statement[1].length()-1);
			}
		}
		if(splitted_statement.size() == 3){
			if(splitted_statement[2].back() != ')') {return false;}
		}
		key = splitted_statement[1];
	}
	switch (splitted_statement.size()) {
		case 1:
			if(find(commands_without_args.begin(), commands_without_args.end(),command)!=commands_without_args.end()){
				return true;
			}
			return false;
		case 2:
			if(find(commands_with_1_arg.begin(), commands_with_1_arg.end(),command)!=commands_with_1_arg.end()){
				if(command == "insert" || command == "connect" || is_valid_key(key)){
					return true;
				}
			}
			return false;
		case 3:
			if(find(commands_with_2_arg.begin(), commands_with_2_arg.end(),command)!=commands_with_2_arg.end()){
				if(is_valid_key(key)){
					return true;
				}
			}
			return false;
		default:
			return false;
			break;
	}

}

vector<string> split_by_delimiter(string str, string delimiter){
    vector<string>result;
  	int pos = str.find(delimiter);
		string first = str.substr(0,pos);
		string second = str.substr(pos+1);
		if(first.size() > 0){
			result.push_back(first);
		}
		if(second != first && second.size() > 0){
			result.push_back(second);
		}
		return result;
}

vector<string> split_statement(string statement){
		vector<string> result;
		vector<string> empty_result;
		result = split_by_delimiter(statement,"(");
		if(result.size() == 1){
			return result;
		}
		vector<string> temp = split_by_delimiter(result[1],",");
		result[1] = temp[0].substr(0,temp[0].size());
		if(temp.size() == 1){
			return result;
		}
		result[1] = temp[0].substr(0,temp[0].size());
		if(temp.size() == 2){
			result.push_back(temp[1].substr(0,temp[1].size()));
			return result;
		}
		return empty_result;
}

int new_socket_connect(string socket_path){
	int fd;
	if(socket_path == ""){
		socket_path = "/tmp/db.tuples.sock";
	}
	int path_length = socket_path.length();
	char temp_path[path_length +1 ];
	strncpy(temp_path,socket_path.c_str(),path_length);
	fd = connection_attempt(temp_path);
	return fd;
}

int disconnect(int fd){
	if(fd != -1){
	close(fd);
	}
}
void quit(int fd){
	cout << "Cerrando proceso cliente" << endl;
	if (fd!=NULL && fd!=-1){
		disconnect(fd);
	}
	exit(-1);
}

string command_code(string command){
	if(command == "connect"){
		return "1";
	}else if(command == "disconnect"){
		return "2";
	}else if(command == "quit"){
		return "3";
	}else if(command == "insert"){
		return "4";
	}else if(command == "get"){
		return "5";
	}else if(command == "peek"){
		return "6";
	}else if(command == "update"){
		return "7";
	}else if(command == "delete"){
		return "8";
	}else if(command == "list"){
		return "9";
	}
}


string exec_statement(string user_input){
	vector<string> splitted_input = split_statement(user_input);
	if(splitted_input.size() > 0 && splitted_input.size() < 4 && validate_statement(splitted_input)){
		switch (splitted_input.size()) {
			case 1:
				return ("0 " + command_code(splitted_input[0]));
			case 2:
				return ("0 " + command_code(splitted_input[0]) + " " +  splitted_input[1].substr(0,splitted_input[1].length()-1));
			case 3:
				return ("0 " + command_code(splitted_input[0]) + " " +  splitted_input[1] + " " +  splitted_input[2].substr(0,splitted_input[2].length()-1));
		}
	}
	return "Sentencia Invalida";
}

int main(int argc, char** argv) {
	char *socket_path = "/tmp/db.tuples.sock";
	struct sockaddr_un addr;
	char buf[100];
	int fd,rc, opt;
	bool is_connected;

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


		fd = connection_attempt(socket_path);
		if(fd != -1){
			is_connected = true;
		}
		else{
			is_connected = false;
		}
		bool read_mode = false;
		while(1){

			while(!is_connected){

			cout << "Modo escritura client" << endl;
			while( (rc=read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
				string temp1(buf);
				string user_input1 = temp1.substr(0,rc-1);
				string client_output1 = exec_statement(user_input1);
				int pos = client_output1.find("1");
				string new_path = client_output1.substr(pos);
				cout << "path:"<< new_path << '\n';
				if(client_output1 == "0 1"){
					fd = new_socket_connect("");
					std::cout << "fd:"<<fd << '\n';
					is_connected = true;
				}else if(client_output1 == "0 3"){
					fd = new_socket_connect(new_path);
					is_connected = true;
				}
				if(client_output1 == "Sentencia Invalida"){
					std::cout << "Error: " << client_output1 << endl;
				}else if(client_output1.substr(0,pos+1) != "0 1"){
					std::cout << "Error: Sentencia Invalida" << endl;
				}
			}
		}

			while(!read_mode && is_connected){

			cout << "Modo escritura client" << endl;
			while( (rc=read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
				string temp(buf);
				string user_input = temp.substr(0,rc-1);
				string client_output = exec_statement(user_input);
				if(client_output == "0 2"){
					disconnect(fd);
				}else if(client_output == "0 3"){
					quit(fd);
				}
				if(client_output == "Sentencia Invalida"){
					std::cout << "Error: " << client_output << '\n';
				}
				else{
					rc = client_output.length();
					const void * temp_buf = client_output.c_str();
					if (write(fd, temp_buf, rc) != rc) {
						if (rc > 0){
							fprintf(stderr,"partial write");
						}else {
							perror("write error");
							exit(-1);
			      		}
			    	}

			    if(client_output.front()=='0'){
						read_mode = true;
						break;
					}
				}
			}
		}

			while(read_mode && is_connected){
			cout << "Modo lectura client" << endl;
			if ( (rc=read(fd,buf,sizeof(buf))) > 0) {
				string server_output(buf);
				cout << server_output.substr(2,rc-2) << endl;
				//printf("read %u bytes: %.*s\n", rc, rc, buf);
			}
			if (rc == -1) {
				perror("read");
			exit(-1);
			}
			else if (rc == 0) {
				printf("EOF\n");
				close(fd);
			}
			if(*buf=='0'){
				read_mode = false;
			}
		}
		}

	return 0;
}
