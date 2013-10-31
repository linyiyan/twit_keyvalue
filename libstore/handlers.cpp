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


using namespace std;

#include "twit_hash.h"
#include "config.h"
#include "storage_server_manager.h"
#include "handlers.h"


void send_http_req(const server_config& server , const map<string,string>& params , decltype(twit_store_opr_resp_handler) cb , void* cbArg){
	string uri = "/?";
	for(auto p : params){
		uri += p.first + "=" + p.second + "&";
	}
	uri = uri.substr(0,uri.size()-1);
	
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

void set(storage_server_manager& mgr , string szKey , string szValue){
	unsigned int hash = twit_hash(szKey.c_str() , szKey.size()*sizeof(char) , 0);
	server_config store_server = mgr.getServerByHashCode(hash);
	
	map<string,string> params = {{"method","set"} , {"key",to_string(hash)} , {"value" , szValue}};
	
	send_http_req(store_server , params , twit_store_opr_resp_handler , NULL);
}

void get(storage_server_manager& mgr , string szKey)
{
	unsigned int hash = twit_hash(szKey.c_str() , szKey.size()*sizeof(char) , 0);
	server_config store_server = mgr.getServerByHashCode(hash);
	
	map<string,string> params = {{"method","get"} , {"key",to_string(hash)}};
	
	send_http_req(store_server , params , twit_store_opr_resp_handler , NULL);
}


void twit_server_http_req_handler(evhttp_request *req, void *arg) {
   
  ostringstream oss;
    
   
  string szUri = evhttp_request_uri(req);
  oss<<"uri="<<szUri<<endl;    
  
  
  oss<<"req uri="<<req->uri<<endl;
  
  
  string szDecodedUri = evhttp_decode_uri(szUri.c_str());
  oss<<"decoded_uri"<<szDecodedUri<<endl;
  
  evkeyvalq params;
  evhttp_parse_query(szDecodedUri.c_str(), &params);
  
  
  
  string method = evhttp_find_header(&params , "method");
  oss<<"method="<<method<<endl;
  
  
  
  if(method=="get"){
    string szKey = evhttp_find_header(&params , "key");
    oss<<"key: "<<szKey<<endl;
    oss<<"hashed key: "<<twit_hash(szKey.c_str() , szKey.size()*sizeof(char) , 0)<<endl;
    }
  else if(method=="set"){
    string szKey = evhttp_find_header(&params , "key");
    oss<<"key: "<<szKey<<endl;
    oss<<"hashed key: "<<twit_hash(szKey.c_str() , szKey.size()*sizeof(char) , 0)<<endl;
    string szVal = evhttp_find_header(&params , "value");
    oss<<"value: "<<szVal<<endl;
    }
  else{
    oss<<"unexpected method"<<endl;
  }


  //HTTP header
  evhttp_add_header(req->output_headers, "Server", "0.0.0.0");
  evhttp_add_header(req->output_headers, "Content-Type", "text/plain; charset=UTF-8");
  evhttp_add_header(req->output_headers, "Connection", "close");
  
  struct evbuffer *buf;
  buf = evbuffer_new();
  evbuffer_add_printf(buf, "It works!\n%s\n", oss.str().c_str());
  evhttp_send_reply(req, HTTP_OK, "OK", buf);
  evbuffer_free(buf);
    
}



void twit_store_setup_resp_handler(evhttp_request *req, void *arg) {
	
	if(req->kind==EVHTTP_RESPONSE){
		printf("it's response\n");
	}
	if (req == NULL) {
        printf("timed out!\n");
    } else if (req->response_code == 0) {
    	storage_server_manager::storage_server_list_initialized = 1;
        printf("connection refused!\n");
    } else if (req->response_code != 200) {
    	storage_server_manager::storage_server_list_initialized = 1;
        printf("error: %u %s\n", req->response_code, req->response_code_line);
    } else {
    	storage_server_manager::storage_server_list_initialized = 2;
    	evbuffer *input = req->input_buffer;
    	char buf[1024];
    	
    	ostringstream oss;
    	int n;
    	while ((n = evbuffer_remove(input, buf, sizeof(buf))) > 0) {
    		oss<<buf;
       		// printf("%s\n" , buf);
    	}
    	
    	storage_server_manager::instance().init_servers(oss.str());
        printf("success : %u %s\n", req->response_code, oss.str().c_str());
    }	
}

void twit_store_opr_resp_handler(evhttp_request *req, void *arg) {
	evbuffer *input = req->input_buffer;
	char buf[1024];
    	
	ostringstream oss;
	int n;
	while ((n = evbuffer_remove(input, buf, sizeof(buf))) > 0) {
		oss<<buf;
       		// printf("%s\n" , buf);
	}
	
	
	
	
	printf("success : %u %s\n", req->response_code, oss.str().c_str());
}









/*
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
    string szOpr = "get";
    req = evhttp_request_new(twit_store_opr_resp_handler, (void*)szOpr.c_str());
    evhttp_add_header(req->output_headers, "Host", addr);
    evhttp_add_header(req->output_headers, "Content-Length", "0");
    
    ostringstream oss;
    oss<<"/?method=get"<<"&"<<"key="<<hash;
    evhttp_make_request(conn, req, EVHTTP_REQ_GET, oss.str().c_str());
	
	event_base_dispatch(base);
}

*/





















