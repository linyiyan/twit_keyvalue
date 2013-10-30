#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     //for getopt, fork
#include <string.h>     //for strcat
//for struct evkeyvalq
#include <sys/queue.h>
#include <event.h>
//for http
#include <evhttp.h>

#include <string>
#include <vector>
#include <iostream>
#include <sstream>

using namespace std;

#include "twit_hash.h"
#include "handlers.h"
#include "config.h"
#include "storage_server_manager.h"



void set(storage_server_manager& mgr , string szKey , string szValue)
{
	unsigned int hash = twit_hash(szKey.c_str() , szKey.size()*sizeof(char) , 0);
	server_config store_server = mgr.getServerByHashCode(hash);
	
	event_base* base = event_init();
	const char *addr = store_server.address.c_str();
    unsigned int port = store_server.port;
	
	evhttp_connection *conn;
	evhttp_request *req;
	
	conn = evhttp_connection_new(addr, port);
    evhttp_connection_set_timeout(conn, 5);
    req = evhttp_request_new(twit_store_opr_resp_handler, (void *)conn);
    evhttp_add_header(req->output_headers, "Host", addr);
    evhttp_add_header(req->output_headers, "Content-Length", "0");
    
    ostringstream oss;
    oss<<"/?method=set"<<"&"<<"key="<<hash<<"&"<<"value="<<szValue;
    evhttp_make_request(conn, req, EVHTTP_REQ_GET, oss.str().c_str());
	
	event_base_dispatch(base);
}

void get(storage_server_manager& mgr , string szKey)
{
	unsigned int hash = twit_hash(szKey.c_str() , szKey.size()*sizeof(char) , 0);
	server_config store_server = mgr.getServerByHashCode(hash);
	
	event_base* base = event_init();
	const char *addr = store_server.address.c_str();
    unsigned int port = store_server.port;
	
	evhttp_connection *conn;
	evhttp_request *req;
	
	conn = evhttp_connection_new(addr, port);
    evhttp_connection_set_timeout(conn, 5);
    req = evhttp_request_new(twit_store_opr_resp_handler, (void *)conn);
    evhttp_add_header(req->output_headers, "Host", addr);
    evhttp_add_header(req->output_headers, "Content-Length", "0");
    
    ostringstream oss;
    oss<<"/?method=get"<<"&"<<"key="<<hash;
    evhttp_make_request(conn, req, EVHTTP_REQ_GET, oss.str().c_str());
	
	event_base_dispatch(base);
}

int main(int argc, char *argv[]) {
    
  server_config twit_server_config("0.0.0.0" , 8080 , 120);

  storage_server_manager& mgr = storage_server_manager::instance();
  mgr.init();
 
  set(mgr , "t3" , "test1");
  set(mgr , "t2" , "test2");
  
  get(mgr,"t2");
  get(mgr,"t3");
  
  event_init();
  
  evhttp *httpd 
    = evhttp_start(twit_server_config.address.c_str(), 
                   twit_server_config.port);
  
  evhttp_set_timeout(httpd, twit_server_config.timeout);
  
  
  evhttp_set_gencb(httpd, twit_server_http_req_handler, NULL);
  
  event_dispatch();
  
  evhttp_free(httpd); 
  
  return 0;
}
