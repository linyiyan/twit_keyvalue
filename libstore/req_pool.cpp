#include <vector>
#include <algorithm>
#include <event.h>
#include <iostream>
//for http
#include <evhttp.h>

#include <pthread.h>

using std::cout;
using std::endl;
using std::vector;
using std::find_if;

#include "req_pool.h"

httpreq_pool& httpreq_pool::instance() {
	static httpreq_pool inst;
	return inst;
}

httpreq_pool_entry& httpreq_pool::next_avail_req_entry() {
	auto pos = find_if(pool.begin(), pool.end(),
			[](httpreq_pool_entry &entry) {return entry.available;});
	return *pos;
}

void httpreq_pool::register_cb(void (*cb)(evhttp_request *, void *)) {
	this->cb = cb;
}

void httpreq_pool::init(unsigned int req_num) {
	for (int i = 0; i < req_num; i++) {
		evhttp_request *req = evhttp_request_new(this->cb, NULL);

		evhttp_add_header(req->output_headers, "Host", "0.0.0.0");
		evhttp_add_header(req->output_headers, "Content-Length", "0");

		bool avail = true;

		evhttp_request_own(req);

		this->pool.push_back(httpreq_pool_entry(avail, req));
	}
}
