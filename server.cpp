#include <iostream>
#include <memory>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "util.h"

using namespace std;

// Almacenamiento KV
KVStore db;

// Función insert por ahora sin validación de las claves
int insert(unsigned long key, string data){
	vector<byte> vdata(data.begin(),data.end());
	Value value = { data.size(), vdata};
	db.insert(std::pair<unsigned long, Value>(key, value));
	return 0;
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

	// Uso elemental del almacenamiento KV:

	// Creamos un arreglo de bytes a mano
	//byte data[] = { 0x0F, 0x01, 0x01, 0x01, 0x01 };
	string data = "abcde";
	// Luego un vector utilizando el arreglo de bytes
	//vector<byte> vdata(data, data + sizeof(data));
	//vector<byte> vdatastr(datastr.begin(),datastr.end());

	// Creamos el valor
	//Value val = { sizeof(data), vdata };
	//Value valstr = { datastr.size(), vdatastr};

	// Insertamos un par clave, valor directamente
	// en el mapa KV

	// Nota: Debiera diseñarse una solución más robusta con una interfaz
	// adecuada para acceder a la estructura.
	//db.insert(std::pair<unsigned long, Value>(1000, valstr));

	insert(1000,data);

	// Imprimir lo que hemos agregado al mapa KV.
	cout << db[1000].size << " " << (int) db[1000].data[0] << endl;
	// esto de abajo es un ejemplo de recuperación del string que está en el arreglo de bytes
	string str_saved(db[1000].data.begin(),db[1000].data.end());
	cout<< (str_saved.data()) << " size:"<< str_saved.size() << endl;

	return 0;
}
