#include <vector>
#include <cstddef>
#include <iostream>
#include <algorithm>
#include <functional>
#include <iterator>
#include <string>

using std::string;
using std::vector;
using std::ostream;
using std::transform;
using std::ostream_iterator;
using std::pair;

#include "config.h"
#include "chunk.h"
#include "slab.h"

void slabs_init(const slab_config& config, vector<Slab>& slabs){

  for(auto p : config.slab_settings){
    slabs.push_back(Slab(p.first,p.second));
  }
}

void Slab::print_chunks(ostream& out){
	transform(chunks.begin() , chunks.end() , ostream_iterator<string>(out , "\n") , mem_fun_ref(&Chunk::to_string));
}


