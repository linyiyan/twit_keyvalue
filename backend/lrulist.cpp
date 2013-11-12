#include <unordered_map>
#include <vector>
#include <list>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <pthread.h>
#include <iterator>

extern pthread_mutex_t lru_lock;

using namespace std;

#include "lrulist.h"


LRUList::LRUList(unsigned int chunk_num){
  int init = 0;
  lst.resize(chunk_num);
  generate(lst.begin() , lst.end() , [&init](){return init++;});
}


void LRUList::lru_push_front(unsigned int elem){
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

unsigned int LRUList::get_oldest(){
  pthread_mutex_lock(&lru_lock);
  if(!lst.empty()){
    int last_elem = lst.back();
    pthread_mutex_unlock(&lru_lock);
    return last_elem;
  }
  else {
    pthread_mutex_unlock(&lru_lock);
    return 0; 
  }  
}


void LRUList::drag_to_front(unsigned int& elem){
  pthread_mutex_lock(&lru_lock);
  
  auto pos = find_if(lst.begin() , lst.end() , [&elem](unsigned int& t){return elem==t;});
  if(pos != lst.end()){
    lst.erase(pos);	
  }
  
  lst.push_front(elem);
  
  pthread_mutex_unlock(&lru_lock);
}

/*
void LRUList::print_lru_list(ostream& out){
  pthread_mutex_lock(&lru_lock);	
  copy(lst.begin() , lst.end() , ostream_iterator<unsigned int>(out , "  "));
  cout<<endl;
  pthread_mutex_unlock(&lru_lock);
}
*/


































