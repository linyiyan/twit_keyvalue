#ifndef LRULIST_H
#define LRULIST_H

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




#endif

