#ifndef SLAB_H
#define SLAB_H


class Slab{
private:
  vector<Chunk> chunks;

public:
	Slab() {}
	Slab(unsigned int chunks_num , unsigned int chunk_size){
		for(unsigned int i=0 ; i<chunks_num ; i++){
			chunks.push_back(Chunk(chunk_size));
		}
	}

    Chunk& operator[] (unsigned int i) {return chunks[i];}
	const Chunk& operator[] (unsigned int i) const {return chunks[i];}
	
	size_t size() {return chunks.size();}
	
	void print_chunks(ostream& out);
};

void slabs_init(const slab_config& config , vector<Slab>& slabs);

#endif
