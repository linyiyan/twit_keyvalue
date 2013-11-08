#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     //for getopt, fork
#include <string.h>     //for strcat
//for struct evkeyvalq
#include <sys/queue.h>
#include <event.h>
//for http
#include <evhttp.h>

#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <unordered_map>

using namespace std;

#include "twit_hash.h"
#include "config.h"
#include "manager.h"
#include "handlers.h"

#include "req_pool.h"

#include "tcache.h"


int main(int argc, char *argv[]) {

	server_config twit_server_config("0.0.0.0", 8080, 120);

	server_manager& mgr = server_manager::instance();
	mgr.init();

	event_base* base = event_init();

	evhttp *httpd = evhttp_start(twit_server_config.address.c_str(), twit_server_config.port);

	evhttp_set_timeout(httpd, twit_server_config.timeout);

	evhttp_set_cb(httpd, "/", twit_server_http_req_handler, NULL);

	httpreq_pool& req_pool = httpreq_pool::instance();
	req_pool.register_cb(twit_store_opr_resp_handler);
	req_pool.init(3);

	event_base_dispatch(base);

	evhttp_free(httpd);

	return 0;
}
