#include <map>
#include <vector>

using namespace std;

typedef unsigned char byte;

// Esta estructura guarda el valor en la estructura K-V
typedef struct {
	size_t size;
	vector<byte> data;
} Value;

// Definicion de la estructura K-V utilizada
// Clave es numerica y valor es dado por estructura Value.
typedef map<unsigned long, Value> KVStore;
