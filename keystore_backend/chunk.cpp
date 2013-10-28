#include <sstream>
#include <string>


using std::ostringstream;
using std::string;

using namespace std;

#include "chunk.h"

string Chunk::to_string(){
	ostringstream oss;
	oss<<(void*)data<<" data: "<<data<<" | size: "<< size << " | assoc key: " << assoc_key;
	return oss.str();
}
