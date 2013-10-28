#include <unordered_map>
#include <vector>
#include <list>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <pthread.h>

pthread_mutex_t lru_lock;

using namespace std;

#include "chunk.h"
#include "slab.h"
#include "lrulist.h"
#include "lrulist.cpp"
#include "global.h"





int main()
{
	pthread_mutex_init(&lru_lock, NULL); 

	slabs_init(slabs);

	cout<<slabs.size()<<endl;
	
	return 0;
}
