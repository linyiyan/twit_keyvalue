#ifndef CHUNK_H
#define CHUNK_H

class Chunk{
	char* data;
	unsigned int size;
	unsigned int assoc_key; // the associated global hash key in key_to_slab.

	public:
	Chunk() : data(NULL), size(0){}
	Chunk(unsigned int s) : data(new char[s]) , size(s) , assoc_key(0){}
	
	unsigned int get_hash() {return assoc_key;}
	void set(char* d , unsigned int hashed_code) {data = d; assoc_key = hashed_code;} 
};

#endif
