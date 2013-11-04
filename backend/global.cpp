#include <unordered_map>
#include <vector>
#include <list>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <pthread.h>

using namespace std;

#include "config.h"
#include "chunk.h"
#include "slab.h"
#include "lrulist.h"

extern vector<Slab> slabs;
extern unordered_map<unsigned int , pair<unsigned int, unsigned int>> key_to_slab;
extern unordered_map<unsigned int , LRUList> slab_lru;

unsigned int slab_index_by_chunk_size(unsigned int size){
  slab_config cfg = slab_config("setting");

  for(int i=0 ; i<cfg.slab_settings.size() ; i++){
    if(i==0 && size<cfg.slab_settings[i].second) return 0;
    else if(size> cfg.slab_settings[i-1].second && size < cfg.slab_settings[i].second) return i;
  }
  
  return -1;
}

void init_slab_lru(const slab_config& config){

  for(int i=0 ; i<config.slab_settings.size() ; i++){
    slab_lru[i] = LRUList(config.slab_settings[i].first);
  }
}


Chunk get(unsigned int key){
	if(key_to_slab.find(key)==key_to_slab.end()){
		return Chunk();
	}else{
		auto slab_pair = key_to_slab[key];

		// update lru 
		LRUList& lst = slab_lru[slab_pair.first];
		lst.drag_to_front(slab_pair.second);
		
		return slabs[slab_pair.first][slab_pair.second];	
	}
}

void set(unsigned int key , char* value , unsigned int size){

  unsigned int slab_index = slab_index_by_chunk_size(size);
  /* if(size<20){
		slab_index = 0;		
	}else if(size<40){
		slab_index = 1;
	}
  */

	// get the oldest index from lru
	LRUList lrulst = slab_lru[slab_index];
	unsigned int oldest_index = lrulst.get_oldest();
	
	cout<<"oldest index: "<<oldest_index<<endl;

	// get the associated global key_to_slab key for the oldest chunk;
	unsigned int hashed_key = slabs[slab_index][oldest_index].get_hash();
	// remove it from global key_to_slab map
	if(hashed_key!=0) // if the chunk is not free
		key_to_slab.erase(hashed_key);
	
	// replace the oldest value with new value and the global hashed key
	slabs[slab_index][oldest_index].set(value , key);

    slabs[slab_index].print_chunks(cout);

	// update key_to_slab
	key_to_slab.insert(make_pair(key , make_pair(slab_index , oldest_index)));

	// update lru
	LRUList& lst = slab_lru[slab_index];
	lst.drag_to_front(oldest_index);
}

void replace(unsigned int key , char* value , unsigned int size){
  auto slab_pair = key_to_slab[key];

  // update lru 
  LRUList& lst = slab_lru[slab_pair.first];
  lst.drag_to_front(slab_pair.second);

  slabs[slab_pair.first].print_chunks(cout);
  
  slabs[slab_pair.first][slab_pair.second].set(value , size);
}
