#ifndef GLOBAL_H
#define GLOBAL_H

vector<Slab> slabs;
unordered_map<unsigned int , pair<unsigned int, unsigned int>> key_to_slab;
unordered_map<unsigned int , LRUList<unsigned int>> slab_lru;

#endif
