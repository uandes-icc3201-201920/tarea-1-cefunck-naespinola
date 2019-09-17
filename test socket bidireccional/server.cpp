#include <iostream>
#include <memory>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ctime>
#include <math.h>
#include "util.h"

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>


using namespace std;

// Almacenamiento KV
KVStore db;


int main(int argc, char** argv) {

	struct sockaddr_un addr;
  	char buf[100];
  	int fd,cl,rc; //cl es client cocket, fd es server socket
  	//int bind, listen, server_socket, accept, read; //posibles variables para sacar lo que esta dentro de los if

	char *socket_path = "/tmp/db.tuples.sock"; //saque el punto que estaba al final

	int opt;

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

    //Se crea un socket de tipo SCK_STREAM y protocolo 0
    if ( (fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
	    perror("socket error");
	    exit(-1);
  	}

  	memset(&addr, 0, sizeof(addr));
  	addr.sun_family = AF_UNIX;
	  strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);
    unlink(socket_path);

    //Establecer vinculacion con la dirección local del socket
	if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
	    perror("bind error");
	    exit(-1);
	}

	//Escuchar conexion de cliente
	if (listen(fd, 5) == -1) { //permite una cola de 5 conexiones
	    perror("listen error");
	    exit(-1);
	}
	cout << "socket listening..." << endl;

	if ( (cl = accept(fd, NULL, NULL)) == -1) {
	perror("accept error");
	}


	//Intento de read

	bool lectura = false;

	while (1) {

		while(!lectura){
			cout << "Entre al modo escritura en server" << endl;
			while( (rc=read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
				if (write(cl, buf, rc) != rc) {
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
			cout << "Entre al modo lectura en server" << endl;
			if ( (rc=read(cl,buf,sizeof(buf))) > 0) {
				printf("read %u bytes: %.*s\n", rc, 100, buf);
			}
			if (rc == -1) {
				perror("read");
			exit(-1);
			}
			else if (rc == 0) {
				printf("EOF\n");
				close(cl);
			}
			if(*buf=='0'){
				lectura = false;
			}
		}

	}

	cout << socket_path << endl;

	return 0;
}
