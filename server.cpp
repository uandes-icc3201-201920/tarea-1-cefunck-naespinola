#include <iostream>
#include <memory>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ctime>
#include <math.h>
#include "util.h"

using namespace std;

// Almacenamiento KV
KVStore db;
unsigned long global_key_counter;


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
	throw invalid_argument( "the key entered does not exist in database" );
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
int insert_into_db(unsigned long key, Value value){
	if (peek(key) || !is_valid_key(key)){
		return -1;
	}
	db.insert(pair<unsigned long, Value>(key, value));
	return key;
}

// Función que inserta un dato en la base de datos con clave autogenerada, si
// la operación tiene exito retorna la clave generada.
int insert_into_db(Value value){
	unsigned long new_key = global_key_counter;
	int result = insert_into_db(new_key, value);
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

	Value value1 = string_to_value("primer dato");
	Value value2 = string_to_value("segundo dato");
	Value value3 = string_to_value("tercer dato");
	insert_into_db(1000,value1);
	insert_into_db(1001,value2);
	insert_into_db(1001,value2);
	insert_into_db(value2);
	insert_into_db(value3);
	update_into_db(1001,string_to_value("dato actualizado"));

	cout << db.size() << endl;
	if (delete_from_db(1000) == 0)
		cout << "deleted" << endl;
	else
		cout << "cant delete" << endl;
	cout << db.size() << endl;


	vector<unsigned long> keys = list();
	for(vector<unsigned long>::iterator it = keys.begin(); it != keys.end(); ++it) {
		unsigned long key = *it;
		Value value = get(key);
		string str_value = value_to_string(value);
		cout << "clave:" << *it << " valor:" << str_value << endl;
	}

	return 0;
}
