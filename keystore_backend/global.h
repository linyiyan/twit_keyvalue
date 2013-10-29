#ifndef GLOBAL_H
#define GLOBAL_H

vector<map<string,string>> storage_servers;

vector<Slab> slabs;
unordered_map<unsigned int , pair<unsigned int, unsigned int>> key_to_slab;
unordered_map<unsigned int , LRUList> slab_lru;

Chunk get(unsigned int key);
void set(unsigned int key , char* value , unsigned int size);

void init_slab_lru(unsigned int chunk_num);

#endif
