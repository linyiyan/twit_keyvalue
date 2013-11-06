#ifndef GLOBAL_H
#define GLOBAL_H


/*
  lease-based cache entry
*/
struct tcached_entry{
  unsigned long duration;
  unsigned long last_updated;

  char* data;
  unsigned int size;
};

/*
  lease-based cache
*/
unordered_map<unsigned int , tcached_entry> tcache;

#endif
