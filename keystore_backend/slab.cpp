#include <vector>
#include <cstddef>

using std::vector;

#include "chunk.h"
#include "slab.h"

void slabs_init(vector<Slab>& slabs){
	// only 20bytes and 40bytes chunks.
	// each slab with 20 chunks;

	slabs = {Slab(20 , 20) , Slab(20 , 40)};
}
