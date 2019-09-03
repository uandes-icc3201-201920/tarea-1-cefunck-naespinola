#include <iostream>
#include <memory>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ctime>
#include "util.h"

using namespace std;

// Almacenamiento KV
KVStore db;
unsigned long global_key_counter;

// Función que retorna 1 si la clave ya existe y 0 si no.
int is_an_existing_key(unsigned long new_key, KVStore db){
	for(map<unsigned long, Value>::iterator it = db.begin(); it != db.end(); ++it) {
		if(it->first == new_key){
			return 1;
		}
	}
	return 0;
}

// Función que retorna una clave aleatorea entera entre 10.000 y 1000.
unsigned long random_key(){
	srand((unsigned)time(0));
	unsigned long i;
	i = (rand()%10000)+1000;
	return i;
}

// Función que inicializa la base de datos.
void init_db(){
	global_key_counter = random_key();
}

// Función que inserta un dato en la clave indicada, validando primero si ya
// existiese la clave, en tal caso no la inserta y retorna -1, en caso contrario
// la inserta exitosamente el dato en la clave indicada y retorna la clave.
int insert_into_db(unsigned long key, string data){
	if (is_an_existing_key(key, db)){
		return -1;
	}
	vector<byte> vdata(data.begin(),data.end());
	Value value = { data.size(), vdata};
	db.insert(pair<unsigned long, Value>(key, value));
	return key;
}

// Función que inserta un dato en la base de datos con clave autogenerada, si
// la operación tiene exito retorna la clave generada.
int insert_into_db(string data){
	unsigned long new_key = global_key_counter++;
	return insert_into_db(new_key, data);
}


int main(int argc, char** argv) {

	int sflag = 0;
	int opt;

	// Procesar opciones de linea de comando
    while ((opt = getopt (argc, argv, "s:")) != -1) {
        switch (opt)
		{
			/* Procesar el flag s si el usuario lo ingresa */
			case 's':
				sflag = 1;
				break;
			default:
				return EXIT_FAILURE;
          }
    }

	init_db();
	string data = "abcde";
	cout << insert_into_db(1000,data) << endl;
	cout << insert_into_db(1001,data) << endl;
	cout << insert_into_db(1001,data) << endl;
	// Imprimir lo que hemos agregado al mapa KV.
	cout << db[1000].size << " " << (int) db[1000].data[0] << endl;
	// esto de abajo es un ejemplo de recuperación del string que está en el arreglo de bytes
	string str_saved(db[1000].data.begin(),db[1000].data.end());
	cout<< (str_saved.data()) << " size:"<< str_saved.size() << endl;

	return 0;
}
