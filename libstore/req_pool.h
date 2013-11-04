#ifndef REQ_POOL_H_
#define REQ_POOL_H_

struct httpreq_pool_entry {
	bool available;
	evhttp_request * req;
	pthread_mutex_t pool_entry_lock;

	httpreq_pool_entry() {
	}
	httpreq_pool_entry(bool avail, evhttp_request* r) :
			available(avail), req(r) {
		pthread_mutex_init(&pool_entry_lock, NULL);
	}
};

class httpreq_pool {

private:
	httpreq_pool() {
	}
	httpreq_pool(const httpreq_pool& hp) {
	}
	httpreq_pool& operator=(const httpreq_pool& hp) {
	}

private:
	vector<httpreq_pool_entry> pool;
	void (*cb)(evhttp_request *, void *);

public:
	static httpreq_pool& instance();

	httpreq_pool_entry& next_avail_req_entry();
	void register_cb(void (*cb)(evhttp_request *, void *));
	void init(unsigned int req_num);
};

#endif /* REQ_POOL_H_ */
