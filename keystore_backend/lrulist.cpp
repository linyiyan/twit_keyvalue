#include <unordered_map>
#include <vector>
#include <list>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <pthread.h>

extern pthread_mutex_t lru_lock;

using namespace std;

#include "lrulist.h"

template<class T>
void LRUList<T>::lru_push_front(T elem){
	pthread_mutex_lock(&lru_lock);
	
	auto pos = find(this->lst.begin() , this->lst.end() , elem);
	if(pos==this->lst.end()){
		this->lst.push_front(elem);
		if(this->lst.size() > this->max_size){
			this->lst.pop_back();		
		}
	}
	else{
		this->lst.erase(pos);
		this->lst.push_front(elem);
	}
	
	pthread_mutex_unlock(&lru_lock);
}

template<class T>
T LRUList<T>::get_oldest(){
	if(lst.size()<max_size) return lst.size();
	else {
		if(!lst.empty())
			return lst.back();
		else 
			return 0; // TODO: not generic
	}
}

template<class T>
void LRUList<T>::drag_to_front(T& elem){
	auto pos = find_if(lst.begin() , lst.end() , [&elem](T& t){return elem==t;});
	if(pos != lst.end()){
		lst.erase(pos);	
	}

	lst.push_front(elem);
}
