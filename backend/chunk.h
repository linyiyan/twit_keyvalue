#ifndef CHUNK_H
#define CHUNK_H

#include <cstdlib>
#include <cstring>
#include <string>
using std::string;


class Chunk{
	char* data;
	unsigned int size;
	unsigned int assoc_key; // the associated global hash key in key_to_slab.

    pthread_mutex_t chunk_lock;

	public:
	Chunk() : data(NULL), size(0) , assoc_key(0){
      pthread_mutex_init(&chunk_lock, NULL);
    }
	Chunk(unsigned int s) :  assoc_key(0){
      pthread_mutex_init(&chunk_lock, NULL);
      size=s;
      data = new char[s];
      memset(data , '\0' , s);
    }
	
	unsigned int get_hash() {return assoc_key;}
	void set(char* d , unsigned int hashed_code) {
      
      pthread_mutex_lock(&chunk_lock);
      strcpy(data , d); 
      assoc_key = hashed_code;
      pthread_mutex_unlock(&chunk_lock);

    } 
	
	string to_string();
	
	bool is_valid() {return data!=NULL;}
	
	char* get_data(){return data;}

    unsigned int  get_data_size(){return size;}
};

#endif
