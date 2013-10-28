#include <unordered_map>
#include <vector>
#include <list>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <pthread.h>
#include <string>
#include <sstream>

pthread_mutex_t lru_lock;

using namespace std;

#include "chunk.h"
#include "slab.h"
#include "lrulist.h"
#include "global.h"





int main()
{
	pthread_mutex_init(&lru_lock, NULL); 

	slabs_init(slabs);
	init_slab_lru(20);

	for(int i=0 ; i<21 ; i++){
		ostringstream oss;
		oss<<"test"<<i;
		string szVal = oss.str();
		
		set(i+1 , const_cast<char*>(szVal.c_str()) , szVal.size());
		slab_lru[0].print_lru_list(cout);
		slabs[0].print_chunks(cout);	
		
	}
	
	return 0;
}
