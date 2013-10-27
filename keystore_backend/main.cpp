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
	unsigned int assoc_key; // the associated global hash key in key_to_slab.

	public:
	Chunk() : data(NULL), size(0){}
	Chunk(unsigned int s) : data(new char[s]) , size(s) , assoc_key(0){}
	
	unsigned int get_hash() {return assoc_key;}
	void set(char* d , unsigned int hashed_code) {data = d; assoc_key = hashed_code;} 
};



class Slab{
private:
	vector<Chunk> chunks;

public:
	Slab() {}
	Slab(unsigned int chunks_num , unsigned int chunk_size){
		chunks = vector<Chunk>(chunks_num , Chunk(chunk_size));	
	}
	Chunk& operator[] (unsigned int i) {return chunks[i];}
	const Chunk& operator[] (unsigned int i) const {return chunks[i];}
};

vector<Slab> slabs;


unordered_map<unsigned int , pair<unsigned int, unsigned int>> key_to_slab;

template<class T>
class LRUList{
	private: 
	list<T> lst;
	unsigned int max_size;
	
	public:
	void lru_push_front(T elem);
	T get_oldest();

	void drag_to_front(T& elem); // drag the elem in list to the front.
	
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
}

unordered_map<unsigned int , LRUList<unsigned int>> slab_lru;

void slabs_init(vector<Slab>& slabs){
	// only 20bytes and 40bytes chunks.
	// each slab with 20 chunks;

	slabs = {Slab(20 , 20) , Slab(20 , 40)};
}

Chunk get(unsigned int key){
	if(key_to_slab.find(key)==key_to_slab.end()){
		return Chunk();
	}
	else{
		auto slab_pair = key_to_slab[key];

		// update lru 
		LRUList<unsigned int>& lst = slab_lru[slab_pair.first];
		
		
		return slabs[slab_pair.first][slab_pair.second];	
	}
}

void set(unsigned int key , char* value , unsigned int size){

	unsigned int slab_index = 0;
	if(size<20){
		slab_index = 0;		
	}
	else if(size<40){
		slab_index = 1;
	}

	// get the oldest index from lru
	LRUList<unsigned int> lrulst = slab_lru[slab_index];
	unsigned int oldest_index = lrulst.get_oldest();

	// get the associated global key_to_slab key for the oldest chunk;
	unsigned int hashed_key = slabs[slab_index][oldest_index].get_hash();
	// remove it from global key_to_slab map
	key_to_slab.erase(hashed_key);
	
	// replace the oldest value with new value and the global hashed key
	slabs[slab_index][oldest_index].set(value , key);

	// update key_to_slab
	key_to_slab.insert(make_pair(key , make_pair(slab_index , oldest_index)));

	// update lru
}

int main()
{
	pthread_mutex_init(&lru_lock, NULL); 

	slabs_init(slabs);

	cout<<slabs.size()<<endl;
	
	return 0;
}
