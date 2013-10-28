#include <unordered_map>
#include <vector>
#include <list>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <pthread.h>

using namespace std;

#include "chunk.h"
#include "slab.h"
#include "lrulist.h"
#include "lrulist.cpp"


extern vector<Slab> slabs;
extern unordered_map<unsigned int , pair<unsigned int, unsigned int>> key_to_slab;
extern unordered_map<unsigned int , LRUList<unsigned int>> slab_lru;



Chunk get(unsigned int key){
	if(key_to_slab.find(key)==key_to_slab.end()){
		return Chunk();
	}
	else{
		auto slab_pair = key_to_slab[key];

		// update lru 
		LRUList<unsigned int>& lst = slab_lru[slab_pair.first];
		lst.drag_to_front(slab_pair.second);
		
		return slabs[slab_pair.first][slab_pair.second];	
	}
}

void set(unsigned int key , char* value , unsigned int size){

	unsigned int slab_index = 0;
	if(size<20){
		slab_index = 0;		
	}
	else if(size<40){
		slab_index = 1;
	}

	// get the oldest index from lru
	LRUList<unsigned int> lrulst = slab_lru[slab_index];
	unsigned int oldest_index = lrulst.get_oldest();

	// get the associated global key_to_slab key for the oldest chunk;
	unsigned int hashed_key = slabs[slab_index][oldest_index].get_hash();
	// remove it from global key_to_slab map
	key_to_slab.erase(hashed_key);
	
	// replace the oldest value with new value and the global hashed key
	slabs[slab_index][oldest_index].set(value , key);

	// update key_to_slab
	key_to_slab.insert(make_pair(key , make_pair(slab_index , oldest_index)));

	// update lru
	LRUList<unsigned int>& lst = slab_lru[slab_index];
	lst.drag_to_front(oldest_index);
}
