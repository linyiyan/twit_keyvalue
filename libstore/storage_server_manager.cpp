#include <stdint.h>
#include <limits.h>
#include <vector>
#include <string>
#include <iostream>

#include <event.h>
//for http
#include <evhttp.h>


using std::string;
using std::vector;
using std::cout;
using std::endl;

#include "config.h"
#include "storage_server_manager.h"
#include "handlers.h"

int storage_server_manager::storage_server_list_initialized=0;

storage_server_manager::storage_server_manager(){
	
	
	event_base* base = event_init();
	const char *addr = "localhost";
    unsigned int port = 8090;
	
	evhttp_connection *conn;
	evhttp_request *req;
	
	conn = evhttp_connection_new(addr, port);
    evhttp_connection_set_timeout(conn, 5);
    req = evhttp_request_new(twit_store_setup_resp_handler, (void *)conn);
    evhttp_add_header(req->output_headers, "Host", addr);
    evhttp_add_header(req->output_headers, "Content-Length", "0");
    evhttp_make_request(conn, req, EVHTTP_REQ_GET, "/?method=setup");
	
	event_base_dispatch(base);
	
	cout<<"ddd"<<endl;
	
	
}

storage_server_manager& storage_server_manager::instance(){
	static storage_server_manager inst;
	
	return inst;
}


server_config& storage_server_manager::getServerByHashCode(uint32_t hash_code){

  unsigned int server_num = this->storage_servers.size();
  int index = 0;
  double dhash_code = hash_code;
  for(;index<server_num;index++){
    if(dhash_code>(double)index/(double)(server_num) * (double)UINT_MAX
       && dhash_code<((double)(index+1)/(double)server_num) * (double)UINT_MAX) 
      break;
    
  }
  index = (index+1)%server_num;
  return this->storage_servers[index]; 
}
