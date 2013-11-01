#include <stdint.h>
#include <limits.h>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <functional>
#include <algorithm>
#include <iterator>
#include <unistd.h>
#include <event.h>
//for http
#include <evhttp.h>
#include <map>

using std::map;
using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::istringstream;
using std::ostringstream;
using std::stoi;
using std::transform;
using std::mem_fun_ref;
using std::ostream_iterator;
#include "config.h"
#include "storage_server_manager.h"
#include "handlers.h"

int storage_server_manager::storage_server_list_initialized = 0;

storage_server_manager::storage_server_manager() {

}

void storage_server_manager::init() {
	server_config master = server_config("localhost", 8090, 5);
	map < string, string> params = { {"method","setup"}};
	send_setup_req(master, params, twit_store_setup_resp_handler, NULL);
	int count = 100;
	while (count-- > 1) {
		sleep(1);
		if (storage_server_list_initialized == 2)
			break;
	}
	if (storage_server_list_initialized == 0)
		printf("setup time out\n");
	else if (storage_server_list_initialized == 1)
		printf("setup false\n");
	else
		printf("setup successful\n");
}

storage_server_manager& storage_server_manager::instance() {
	static storage_server_manager inst;
	return inst;
}

void storage_server_manager::init_servers(string sz) {
	istringstream iss(sz);
	string line;
	while (getline(iss, line, '\n')) {
		auto pos = line.find(':');
		if (pos != string::npos) {
			string szIp = line.substr(0, pos);
			string szPort = line.substr(pos + 1);
			storage_servers.push_back(server_config(szIp, stoi(szPort), 50));
		}
	}

}

server_config& storage_server_manager::getServerByHashCode(uint32_t hash_code) {

	unsigned int server_num = storage_servers.size();
	int index = 0;
	double dhash_code = hash_code;
	for (; index < server_num; index++) {
		if (dhash_code
				> (double) index / (double) (server_num) * (double) UINT_MAX
				&& dhash_code
						< ((double) (index + 1) / (double) server_num)
								* (double) UINT_MAX)
			break;

	}
	index = (index + 1) % server_num;
	return storage_servers[index];
}

/*
 void storage_server_manager::init(){
 event_base* base = event_init();
 const char *addr = "localhost";
 unsigned int port = 8090;

 evhttp_connection *conn;
 evhttp_request *req;

 conn = evhttp_connection_new(addr, port);
 evhttp_connection_set_timeout(conn, 5);
 req = evhttp_request_new(twit_store_setup_resp_handler, NULL);
 evhttp_add_header(req->output_headers, "Host", addr);
 evhttp_add_header(req->output_headers, "Content-Length", "0");
 evhttp_make_request(conn, req, EVHTTP_REQ_GET, "/?method=setup");

 event_base_dispatch(base);

 int count = 100;
 while(count-- > 1){
 sleep(1);
 if(storage_server_list_initialized==2) break;
 }
 if(storage_server_list_initialized==0) printf("setup time out\n");
 else if(storage_server_list_initialized==1) printf("setup false\n");
 else printf("setup successful\n");

 }
 */
