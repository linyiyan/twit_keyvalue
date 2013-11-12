#ifndef GLOBAL_H
#define GLOBAL_H


Chunk get(unsigned int key);
void set(unsigned int key , char* value , unsigned int size);
void replace(unsigned int key , char* value , unsigned int size);
void init_slab_lru(const slab_config& config);


void logStatus(string log);



#endif
