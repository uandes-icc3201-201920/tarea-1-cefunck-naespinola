#include <iostream>
#include <memory>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

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

//define el socket path si el usuario coloca el flag -s direccion, sino se coloca por defecto
char * define_socket_path(int argc, char** argv,char *socket_path){
	socket_path = "/tmp/db.tuples.sock";
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
	return socket_path;
}

int contarPalabras(char* palabras)
{
	int contador =0;
	vector<char*> v;
	char * token;
	token = strtok(palabras, " ");
	while(token!=NULL){
		v.push_back(token);
		token = strtok(NULL, " ");
	}
	contador=v.size();
	return contador;
}

//Para cuando el usuario quiera volve a conectar a un socket
int new_socket_connect(char* input, char *socket_path){
	int fd;
	int n = contarPalabras(input);
	switch(n){
		case 1:
		socket_path = "/tmp/db.tuples.sock";
		break;
		case 2:
		socket_path = input[1];				//no hay control de error si no ingresa bien una direccion de socket
		break;
		default:
		cout <<"ERROR comando invalido"<< endl;
	}
	fd = connection_attempt(socket_path);
	return fd;
}

int disconnect(int fd){
	close(fd);
}
void quit(int fd){
	if (fd!=NULL){
		disconnect(fd);
	}
	exit(-1);
}

int main(int argc, char** argv) {
	char *socket_path
	struct sockaddr_un addr;
	char buf[100];
	int fd,rc, opt;

	socket_path = define_socket_path(argc, argv);

	fd = connection_attempt(socket_path);
	bool read_mode = true;
	while(1){

		while(!read_mode){

			cout << "Modo escritura client" << endl;
			while( (rc=read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
				if (write(fd, buf, rc) != rc) {
					if (rc > 0){
						fprintf(stderr,"partial write");
					}else {
						perror("write error");
						exit(-1);
		      		}
		    	}

		    if(*buf=='0'){
					read_mode = true;
					break;
				}
			}
		}

		while(read_mode){
			cout << "Modo lectura client" << endl;
			if ( (rc=read(fd,buf,sizeof(buf))) > 0) {
				printf("read %u bytes: %.*s\n", rc, rc, buf);
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
