#include <unordered_map>
#include <vector>
#include <list>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <pthread.h>
#include <string>
#include <sstream>

#include <event.h>
//for http
#include <evhttp.h>

pthread_mutex_t lru_lock;

using namespace std;

#include "chunk.h"
#include "slab.h"
#include "lrulist.h"
#include "global.h"



void libstore_http_req_handler(evhttp_request *req, void *arg) {
	
	string szUri = evhttp_request_uri(req);
	string szDecodedUri = evhttp_decode_uri(szUri.c_str());
	evkeyvalq params;
    evhttp_parse_query(szDecodedUri.c_str(), &params);
    string method = evhttp_find_header(&params , "method");
    
    
    ostringstream oss;
    if(method=="get"){
    	oss<<"Ok, you can get."<<endl;
    	cout<<"want to get"<<endl;
    	
    	string szKey = evhttp_find_header(&params , "key");
    	unsigned int key = stol(szKey);
    	
    	Chunk ck = get(key);
    	if(!ck.is_valid()) oss<<"But data not found."<<endl;
    	else oss<<"Here is the data: "<<ck.get_data()<<endl;
    }
    else if(method=="set"){
    	oss<<"Ok, you can set";
    	
    	string szKey = evhttp_find_header(&params , "key");
    	unsigned int key = stol(szKey);
    	
    	string szVal = evhttp_find_header(&params , "value");
    	
    	set(key , const_cast<char*>(szVal.c_str()) , szVal.size());
    	cout<<"want to set"<<endl;
    }
    else{
    	oss<<"Oops, method not supported";
    	cout<<"unexpected method"<<endl;
    }
    
    evhttp_add_header(req->output_headers, "Server", "0.0.0.0");
  	evhttp_add_header(req->output_headers, "Content-Type", "text/plain; charset=UTF-8");
  	evhttp_add_header(req->output_headers, "Connection", "close");
  	
  	
  	struct evbuffer *buf;
  	buf = evbuffer_new();
  	evbuffer_add_printf(buf, "It works!\n%s\n", oss.str().c_str());
 	evhttp_send_reply(req, HTTP_OK, "OK", buf);
  	evbuffer_free(buf);
    
}




int main(int argc, char **argv)
{
	pthread_mutex_init(&lru_lock, NULL); 

	slabs_init(slabs);
	init_slab_lru(20);
	
	
	unsigned short 	port = 0;
	if(argc<2){
		cout<<"specify a port"<<endl;
		return 0;
	}
	else{
		string szPort = argv[1];
		port = stoi(szPort);
	}

	
/*
	for(int i=0 ; i<21 ; i++){
		ostringstream oss;
		oss<<"test"<<i;
		string szVal = oss.str();
		
		set(i+1 , const_cast<char*>(szVal.c_str()) , szVal.size());
		slab_lru[0].print_lru_list(cout);
		slabs[0].print_chunks(cout);	
		
	}
*/

	event_init();
	evhttp *httpd = evhttp_start("0.0.0.0", port);
                   
    evhttp_set_timeout(httpd, 120);
	evhttp_set_gencb(httpd, libstore_http_req_handler, NULL);
	
	event_dispatch();
  
  	evhttp_free(httpd); 
  	
  	
  	
  	
  		
	return 0;
}

/*

int success_count = 0;
int failure_count = 0;
time_t start,end;

void _reqhandler(struct evhttp_request *req, void *state)
{
   printf("in _reqhandler. state == %s\n", (char *) state);
   if (req == NULL) {
       printf("timed out!\n");
       failure_count ++;
    } else if (req->response_code == 0) {
    printf("connection refused!\n");
    failure_count++;
    } else if (req->response_code != 200) {
    printf("error: %u %s\n", req->response_code, req->response_code_line);
    failure_count++;
    } else {
    printf("success: %u %s\n", req->response_code, req->response_code_line);
    success_count++;
    }
    evhttp_connection_free((evhttp_connection*)state);
}


event_base* base = event_init();
  	evhttp_connection *conn;
  	evhttp_request *req;
  	conn = evhttp_connection_new("localhost", 8085);
  	evhttp_connection_set_timeout(conn, 5);
  	req = evhttp_request_new(_reqhandler, (void *)conn);
  	event_base_dispatch(base);
  	
  	evhttp_add_header(req->output_headers, "Host", "localhost");
  	evhttp_add_header(req->output_headers, "Content-Length", "0");
  	evhttp_make_request(conn, req, EVHTTP_REQ_GET, "/?method=createUser&usrId=t8");

*/























