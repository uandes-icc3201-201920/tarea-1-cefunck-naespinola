#include <iostream>
#include <memory>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "util.h"

using namespace std;

int client_socket_file_descriptor(char *socket_path){
	int fd;
	struct sockaddr_un addr;

	if ( (fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("socket error");
		exit(-1);
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);

	if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
	  	perror("connect error");
	  	exit(-1);
	}
	return fd;
}

int main(int argc, char** argv) {
	char *socket_path = "/tmp/db.tuples.sock"; //saque el punto que estaba al final
	struct sockaddr_un addr;
	char buf[100];
	int fd,rc, opt;

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

  	fd = client_socket_file_descriptor(socket_path);
	
	bool lectura = true;
	while(1){

		while(!lectura){
			cout << "Entre al modo escritura en client" << endl;
			while( (rc=read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
				if (write(fd, buf, rc) != rc) {
					if (rc > 0) fprintf(stderr,"partial write");
					else {
						perror("write error");
						exit(-1);
		      		}
		    	}
		    	if(*buf=='0'){
				lectura = true;
				break;
				}
			}
		}

		while(lectura){
			cout << "Entre al modo lectura en client" << endl;
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
				lectura = false;
			}
		}
	}

	return 0;	
}