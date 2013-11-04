#include <vector>
#include <cstddef>
#include <iostream>
#include <algorithm>
#include <functional>
#include <iterator>

using std::vector;
using std::ostream;
using std::transform;
using std::ostream_iterator;

#include "chunk.h"
#include "slab.h"

void slabs_init(vector<Slab>& slabs){
	// only 20bytes and 40bytes chunks.
	// each slab with 20 chunks;

	slabs = {Slab(20 , 20) , Slab(20 , 40)};
}

void Slab::print_chunks(ostream& out){
	transform(chunks.begin() , chunks.end() , ostream_iterator<string>(out , "\n") , mem_fun_ref(&Chunk::to_string));
}


