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
#include "manager.h"
#include "handlers.h"

int server_manager::storage_server_list_initialized = 0;

server_manager::server_manager() {
	lease_duration = 0;
}

void server_manager::init() {
	server_config master = server_config("localhost", 8090, 5);
	map<string, string> params = { { "method", "setup" } };
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

server_manager& server_manager::instance() {
	static server_manager inst;
	return inst;
}

void server_manager::init_servers(string sz) {
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

void server_manager::set_lease_duration(unsigned long dur) {
	this->lease_duration = dur;
}

unsigned long server_manager::get_lease_duration() {
	return lease_duration;
}

server_config& server_manager::getServerByHashCode(uint32_t hash_code) {

	unsigned int server_num = storage_servers.size();
	int index = 0;
	double dhash_code = hash_code;
	for (; index < server_num; index++) {
		if (dhash_code > (double) index / (double) (server_num) * (double) UINT_MAX && dhash_code < ((double) (index + 1) / (double) server_num) * (double) UINT_MAX)
			break;

	}
	index = (index + 1) % server_num;
	return storage_servers[index];
}
