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


vector<unsigned long> list(){
	vector<unsigned long> keys;
	for(map<unsigned long, Value>::iterator it = db.begin(); it != db.end(); ++it) {
		keys.push_back(it->first);
	}
	return keys;
}


Value get(unsigned long key){
	Value value = {db[key].size, db[key].data};
	return value;
}

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
	unsigned long new_key = global_key_counter;
	int result = insert_into_db(new_key, data);
	if (result == new_key){
		global_key_counter++;
	}
	return result;
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

	string data1 = "primer dato";
	string data2 = "segundo dato";
	string data3 = "tercer dato";
	insert_into_db(1000,data1);
	insert_into_db(data2);
	insert_into_db(data3);

	Value value_saved = get(1000);
	string str_from_value_saved(value_saved.data.begin(), value_saved.data.end());

	vector<unsigned long> v = list();
	for(vector<unsigned long>::iterator it = v.begin(); it != v.end(); ++it) {
		unsigned long key = *it;
		Value value = get(key);
		string str_value(value.data.begin(), value.data.end());
		cout << "clave:" << *it << " valor:" << str_value << endl;
	}

	return 0;
}
