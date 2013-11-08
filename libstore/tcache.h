#ifndef GLOBAL_H
#define GLOBAL_H


/*
  lease-based cache entry
*/
struct tcache_entry{
  unsigned long duration;
  unsigned long last_updated;

  char* data;
  unsigned int size;

  pthread_mutex_t cache_entry_lock;

  tcache_entry() : duration(0) , last_updated(0) , data(NULL) , size(0){
	  pthread_mutex_init(&cache_entry_lock , NULL);
  }
};

/*
  lease-based cache
*/

class tcache{
private:
	tcache(){}
	tcache(const tcache& tc){}
	tcache& operator= (const tcache& tc){}

public:
	static tcache& instance(){
		static tcache cache = tcache();
		return cache;
	}
private:
	unordered_map<unsigned int , tcache_entry> cache;
public:
	bool mem(unsigned int id);
	tcache_entry& get(unsigned int id);
	void set(unsigned int id , char* data , int size);
};


#endif
