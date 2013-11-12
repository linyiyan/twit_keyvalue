#ifndef LRULIST_H
#define LRULIST_H

class LRUList{
	private: 
	list<unsigned int> lst;
	unsigned int max_size;
	
	public:
	LRUList() {}
	LRUList(unsigned int chunk_num);
	
	void lru_push_front(unsigned int elem);
	unsigned int get_oldest();

	void drag_to_front(unsigned int& elem); // drag the elem in list to the front.
	
	/* void print_lru_list(ostream& out); */
};




#endif

