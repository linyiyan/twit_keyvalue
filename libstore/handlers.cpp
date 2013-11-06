#include <cstdio>
#include <cstdlib>
#include <unistd.h>     //for getopt, fork
//for struct evkeyvalq
#include <sys/queue.h>
#include <event.h>
//for http
#include <evhttp.h>
#include <cstring>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <iostream>

using namespace std;

#include "twit_hash.h"
#include "config.h"
#include "manager.h"
#include "handlers.h"

#include "req_pool.h"

struct twit_server_req {
	string method;
	evhttp_request* req;
	httpreq_pool_entry* pEntry;
	twit_server_req(string m, evhttp_request* r) :
			method(m), req(r), pEntry(NULL) {
	}
};

void send_setup_req(const server_config& server,
		const map<string, string>& params, func_t cb, void* cbArg) {

	string uri = "/?";
	for (auto p : params) {
		uri += p.first + "=" + p.second + "&";
	}
	uri = uri.substr(0, uri.size() - 1);

	event_base* base = event_init();

	const char *addr = server.address.c_str();
	unsigned int port = server.port;

	evhttp_connection *conn = evhttp_connection_new(addr, port);
	evhttp_request *req = evhttp_request_new(cb, cbArg);

	evhttp_add_header(req->output_headers, "Host", addr);
	evhttp_add_header(req->output_headers, "Content-Length", "0");

	evhttp_make_request(conn, req, EVHTTP_REQ_GET, uri.c_str());
	event_base_dispatch(base);
}

void send_http_req(const server_config& server,
		const map<string, string>& params, func_t cb, void* cbArg) {
	string uri = "/?";
	for (auto p : params) {
		uri += p.first + "=" + p.second + "&";
	}
	uri = uri.substr(0, uri.size() - 1);

	const char *addr = server.address.c_str();
	unsigned int port = server.port;

	evhttp_connection *conn = evhttp_connection_new(addr, port);

	httpreq_pool_entry& entry = httpreq_pool::instance().next_avail_req_entry();

	pthread_mutex_lock(&(entry.pool_entry_lock));

	((twit_server_req*) cbArg)->pEntry = &entry;
	entry.req->cb_arg = cbArg;
	entry.available = false;

	pthread_mutex_unlock(&(entry.pool_entry_lock));

	evhttp_make_request(conn, entry.req, EVHTTP_REQ_GET, uri.c_str());
}

void set(server_manager& mgr, string szKey, string szValue,
		evhttp_request *req) {
	unsigned int hash = twit_hash(szKey.c_str(), szKey.size() * sizeof(char),
			0);
	server_config store_server = mgr.getServerByHashCode(hash);

	map<string, string> params = { { "method", "set" },
			{ "key", to_string(hash) }, { "value", szValue } };

	twit_server_req* pq = new twit_server_req("set", req);
	send_http_req(store_server, params, twit_store_opr_resp_handler,
			(void*) pq);
}

void get(server_manager& mgr, string szKey, evhttp_request *req) {
	unsigned int hash = twit_hash(szKey.c_str(), szKey.size() * sizeof(char),
			0);
	server_config store_server = mgr.getServerByHashCode(hash);

	map<string, string> params = { { "method", "get" },
			{ "key", to_string(hash) } };

	twit_server_req* pq = new twit_server_req("get", req);
	send_http_req(store_server, params, twit_store_opr_resp_handler,
			(void*) pq);
}

void incr(server_manager& mgr, string szKey, evhttp_request *req) {
	unsigned int hash = twit_hash(szKey.c_str(), szKey.size() * sizeof(char),
			0);
	server_config store_server = mgr.getServerByHashCode(hash);

	map<string, string> params = { { "method", "incr" }, { "key", to_string(
			hash) } };

	twit_server_req* pq = new twit_server_req("incr", req);
	send_http_req(store_server, params, twit_store_opr_resp_handler,
			(void*) pq);
}

void twit_server_http_req_handler(evhttp_request *req, void *arg) {

	ostringstream oss;

	string szUri = evhttp_request_uri(req);
	oss << "uri=" << szUri << endl;
	cout << "uri=" << szUri << endl;

	oss << "req uri=" << req->uri << endl;

	string szDecodedUri = evhttp_decode_uri(szUri.c_str());
	oss << "decoded_uri" << szDecodedUri << endl;

	evkeyvalq params;
	evhttp_parse_query(szDecodedUri.c_str(), &params);

	string method = evhttp_find_header(&params, "method");
	oss << "method=" << method << endl;

	server_manager& mgr = server_manager::instance();
	if (method == "get") {
		string szKey = evhttp_find_header(&params, "key");
		oss << "key: " << szKey << endl;
		oss << "hashed key: "
				<< twit_hash(szKey.c_str(), szKey.size() * sizeof(char), 0)
				<< endl;
		get(mgr, szKey, req);
	} else if (method == "set") {
		string szKey = evhttp_find_header(&params, "key");
		oss << "key: " << szKey << endl;
		oss << "hashed key: "
				<< twit_hash(szKey.c_str(), szKey.size() * sizeof(char), 0)
				<< endl;
		string szVal = evhttp_find_header(&params, "value");
		oss << "value: " << szVal << endl;
		set(mgr, szKey, szVal, req);
	} else if (method == "incr") {
		string szKey = evhttp_find_header(&params, "key");
		incr(mgr, szKey, req);
	} else {
		oss << "unexpected method" << endl;
	}

}

void twit_store_setup_resp_handler(evhttp_request *req, void *arg) {

	if (req == NULL) {
		printf("timed out!\n");
	} else if (req->response_code == 0) {
		server_manager::storage_server_list_initialized = 1;
		printf("connection refused!\n");
	} else if (req->response_code != 200) {
		server_manager::storage_server_list_initialized = 1;
		printf("error: %u %s\n", req->response_code, req->response_code_line);
	} else {
		server_manager::storage_server_list_initialized = 2;
		evbuffer *input = req->input_buffer;
		char buf[1024];

		ostringstream oss;
		int n;
		while ((n = evbuffer_remove(input, buf, sizeof(buf))) > 0) {
			oss << buf;
		}
        
        string resp_entity = oss.str();
        auto pos = resp_entity.find("|");
        if(pos!=string::npos){
          string szDur = resp_entity.substr(0 , pos);
          resp_entity = resp_entity.substr(pos+1);
        }

		server_manager::instance().init_servers(resp_entity);
		printf("success : %u %s\n", req->response_code, oss.str().c_str());
	}
}

void twit_store_opr_resp_handler(evhttp_request *req, void *arg) {

	evbuffer *input = req->input_buffer;
	char buf[1024];

	ostringstream oss;
	int n;

	while ((n = evbuffer_remove(input, buf, sizeof(buf))) > 0) {
		oss << buf;
	}

	if (arg != NULL) {

		twit_server_req* req = (twit_server_req*) arg;
		string method = req->method;
		evhttp_request* twit_server_resp = req->req;

		evhttp_add_header(twit_server_resp->output_headers, "Server",
				"0.0.0.0");
		evhttp_add_header(twit_server_resp->output_headers, "Content-Type",
				"text/plain; charset=UTF-8");
		evhttp_add_header(twit_server_resp->output_headers, "Connection",
				"close");

		struct evbuffer *evbuf;
		evbuf = evbuffer_new();
		evbuffer_add_printf(evbuf, "%s", oss.str().c_str());
		evhttp_send_reply(twit_server_resp, HTTP_OK, "OK", evbuf);
		evbuffer_free(evbuf);

		pthread_mutex_lock(&(req->pEntry->pool_entry_lock));

		req->pEntry->req = evhttp_request_new(twit_store_opr_resp_handler,
				NULL);
		evhttp_request_own(req->pEntry->req);

		req->pEntry->available = true;
		pthread_mutex_unlock(&(req->pEntry->pool_entry_lock));

	}
}

