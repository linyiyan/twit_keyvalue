#ifndef SLAB_H
#define SLAB_H

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

void slabs_init(vector<Slab>& slabs);

#endif
