#include <unordered_map>
#include <vector>
#include <list>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <pthread.h>

pthread_mutex_t lru_lock;

using namespace std;


class Chunk{
	char* data;
	unsigned int size;

	public:
	Chunk() : data(NULL), size(0){}
	Chunk(unsigned int s) : data(new char[s]) , size(s){}
};

vector<vector<Chunk>> slabs;
unordered_map<unsigned int , pair<unsigned int, unsigned int>> key_to_slab;

template<class T>
class LRUList{
	private: 
	list<T> lst;
	unsigned int max_size;
	
	public:
	void lru_push_front(T elem);
	T get_oldest();
	
};

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
	else return lst.back;
}

unordered_map<unsigned int , LRUList<unsigned int>> slab_lru;

void slab_init(vector<vector<Chunk>>& slabs){
	// only 20bytes and 40bytes chunks.
	// each slab with 20 chunks;

	slabs = {vector<Chunk>(20 , Chunk(20)) , vector<Chunk>(20 ,  Chunk(40))};
}

Chunk get(unsigned int key){
	if(key_to_slab.find(key)==key_to_slab.end()){
		return Chunk();
	}
	else{
		auto slab_pair = key_to_slab[key];
		return slabs[slab_pair.first][slab_pair.second];	
	}
}

void set(unsigned int key , char* value , unsigned int size){
	if(size<20){
		LRUList<unsigned int> lrulst = slab_lru[0];
		
	}
	else if(size<40){
	}
}

int main()
{
	pthread_mutex_init(&lru_lock, NULL); 

	slab_init(slabs);

	cout<<slabs.size()<<endl;
	
	return 0;
}
