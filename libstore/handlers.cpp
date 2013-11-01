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
#include "storage_server_manager.h"
#include "handlers.h"

#include "httpreq_pool.h"





struct twit_server_req{
	string method;
	evhttp_request* req;
	httpreq_pool_entry* pEntry;
	twit_server_req(string m , evhttp_request* r):method(m) , req(r) , pEntry(NULL){}
};	

void send_setup_req(const server_config& server , const map<string,string>& params , func_t cb , void* cbArg){
	
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

void send_http_req(const server_config& server , const map<string,string>& params , func_t cb , void* cbArg){
	cout<<"send beg"<<endl;
	string uri = "/?";
	for(auto p : params){
		uri += p.first + "=" + p.second + "&";
	}
	uri = uri.substr(0,uri.size()-1);
	
	const char *addr = server.address.c_str();
	unsigned int port = server.port;
	
	evhttp_connection *conn = evhttp_connection_new(addr, port);
	
	
	httpreq_pool_entry& entry = httpreq_pool::instance().next_avail_req_entry();
	
	pthread_mutex_lock(&(entry.pool_entry_lock));
	
	((twit_server_req*)cbArg)->pEntry = &entry;
	entry.req->cb_arg = cbArg;
	entry.available = false;
	
	pthread_mutex_unlock(&(entry.pool_entry_lock));  
	
	  
    evhttp_make_request(conn, entry.req, EVHTTP_REQ_GET, uri.c_str());
    cout<<"send end"<<endl;
}

void set(storage_server_manager& mgr , string szKey , string szValue , evhttp_request *req){
	cout<<"in set"<<endl;
	unsigned int hash = twit_hash(szKey.c_str() , szKey.size()*sizeof(char) , 0);
	server_config store_server = mgr.getServerByHashCode(hash);
	
	map<string,string> params = {{"method","set"} , {"key",to_string(hash)} , {"value" , szValue}};
	
	twit_server_req* pq = new twit_server_req("set",req);
	send_http_req(store_server , params , twit_store_opr_resp_handler , (void*)pq);
}

void get(storage_server_manager& mgr , string szKey , evhttp_request *req)
{
	unsigned int hash = twit_hash(szKey.c_str() , szKey.size()*sizeof(char) , 0);
	server_config store_server = mgr.getServerByHashCode(hash);
	
	map<string,string> params = {{"method","get"} , {"key",to_string(hash)}};
	
	twit_server_req* pq = new twit_server_req("get",req);
	send_http_req(store_server , params , twit_store_opr_resp_handler , (void*)pq);
}




void twit_server_http_req_handler(evhttp_request *req, void *arg) 
{
	
	ostringstream oss;
    
  
	string szUri = evhttp_request_uri(req);
	oss<<"uri="<<szUri<<endl;  
	cout<<"uri="<<szUri<<endl;  
  
 
  oss<<"req uri="<<req->uri<<endl;
  
  
  string szDecodedUri = evhttp_decode_uri(szUri.c_str());
  oss<<"decoded_uri"<<szDecodedUri<<endl;
  
  
  evkeyvalq params;
  evhttp_parse_query(szDecodedUri.c_str(), &params);
  
  
  
  string method = evhttp_find_header(&params , "method");
  oss<<"method="<<method<<endl;
  
  
  storage_server_manager& mgr = storage_server_manager::instance();
  if(method=="get"){
    string szKey = evhttp_find_header(&params , "key");
    oss<<"key: "<<szKey<<endl;
    oss<<"hashed key: "<<twit_hash(szKey.c_str() , szKey.size()*sizeof(char) , 0)<<endl;
    std::cout<<"req from twit_server key: "<<szKey<<endl; 
   get(mgr , szKey , req);
    // fget(mgr,szKey);
    }
  else if(method=="set"){
    string szKey = evhttp_find_header(&params , "key");
    oss<<"key: "<<szKey<<endl;
    oss<<"hashed key: "<<twit_hash(szKey.c_str() , szKey.size()*sizeof(char) , 0)<<endl;
    string szVal = evhttp_find_header(&params , "value");
    oss<<"value: "<<szVal<<endl;
    cout<<"in handler8"<<endl;
   	 set(mgr , szKey , szVal , req);
    //gset(mgr , szKey , szVal);
    }
  else{
    oss<<"unexpected method"<<endl;
  }    

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
	cout<<"begin opr"<<endl;

	evbuffer *input = req->input_buffer;
	cout<<"end opr"<<endl;
	char buf[1024];
    	
	ostringstream oss;
	int n;
	
	while ((n = evbuffer_remove(input, buf, sizeof(buf))) > 0) {
		oss<<buf;
       	printf("%s\n" , buf);
	}
	
	
	if(arg!=NULL){
		
		twit_server_req* req = (twit_server_req*) arg;
		string method = req->method;
		evhttp_request* twit_server_resp = req->req;
		
		evhttp_add_header(twit_server_resp->output_headers, "Server", "0.0.0.0");
  		evhttp_add_header(twit_server_resp->output_headers, "Content-Type", "text/plain; charset=UTF-8");
  		evhttp_add_header(twit_server_resp->output_headers, "Connection", "close");
  		
  		struct evbuffer *evbuf;
		evbuf = evbuffer_new();
		evbuffer_add_printf(evbuf, "It works!\n%s\n", oss.str().c_str());
		evhttp_send_reply(twit_server_resp, HTTP_OK, "OK", evbuf);
		evbuffer_free(evbuf);
		
		//cout<<"before free"<<endl;
		pthread_mutex_lock(&(req->pEntry->pool_entry_lock));
		//evhttp_request_free(req->pEntry->req);
		//cout<<"end free"<<endl;
		
		//cout<<"before new"<<endl;
		req->pEntry->req = evhttp_request_new(twit_store_opr_resp_handler, NULL);
		evhttp_request_own(req->pEntry->req);
		//cout<<"end free"<<endl;
		req->pEntry->available = true;
		pthread_mutex_unlock(&(req->pEntry->pool_entry_lock));
		
	}
	
	
	printf("success : %u %s\n", req->response_code, oss.str().c_str()); 
}





/*

void send_http_req(const server_config& server , const map<string,string>& params , func_t cb , void* cbArg){
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
void gset(storage_server_manager& mgr , string szKey , string szValue){
	cout<<"in gset"<<endl;
	unsigned int hash = twit_hash(szKey.c_str() , szKey.size()*sizeof(char) , 0);
	server_config store_server = mgr.getServerByHashCode(hash);
	
	
	evhttp_connection *conn = evhttp_connection_new("0.0.0.0", 8090);
	evhttp_connection_set_timeout(conn, 5);
	
	// evhttp_add_header(greq->output_headers, "Host", "0.0.0.0");
    // evhttp_add_header(greq->output_headers, "Content-Length", "0");
    
     cout<<"after gset"<<endl;
    ostringstream oss;
    oss<<"/?method=set"<<"&"<<"key="<<hash<<"&"<<"value="<<szValue;
    // greq->evcon = conn;
    if(counter++ % 2==0) evhttp_make_request(conn, greq, EVHTTP_REQ_GET, oss.str().c_str());
    if(counter++ % 2==1) evhttp_make_request(conn, greq1, EVHTTP_REQ_GET, oss.str().c_str());
   
}


void fset(storage_server_manager& mgr , string szKey , string szValue)
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
    req = evhttp_request_new(twit_store_opr_resp_handler, NULL);
    evhttp_add_header(req->output_headers, "Host", addr);
    evhttp_add_header(req->output_headers, "Content-Length", "0");
    
    ostringstream oss;
    oss<<"/?method=set"<<"&"<<"key="<<hash<<"&"<<"value="<<szValue;
    evhttp_make_request(conn, req, EVHTTP_REQ_GET, oss.str().c_str());
	evhttp_connection_set_timeout(req->evcon, 600);
	event_base_dispatch(base);
	
	// event_base_dispatch(gbase);
	// event_dispatch();
}

void fget(storage_server_manager& mgr , string szKey)
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
    req = evhttp_request_new(twit_store_opr_resp_handler, NULL);
    evhttp_add_header(req->output_headers, "Host", addr);
    evhttp_add_header(req->output_headers, "Content-Length", "0");
    
    ostringstream oss;
    oss<<"/?method=get"<<"&"<<"key="<<hash;
    evhttp_make_request(conn, req, EVHTTP_REQ_GET, oss.str().c_str());
	
	event_base_dispatch(base);
}

*/




























