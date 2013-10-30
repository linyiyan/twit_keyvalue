#include <cstdio>
#include <cstdlib>
#include <unistd.h>     //for getopt, fork
//for struct evkeyvalq
#include <sys/queue.h>
#include <event.h>
//for http
#include <evhttp.h>

#include <string>
#include <sstream>
#include <vector>


using namespace std;

#include "twit_hash.h"
#include "config.h"
#include "storage_server_manager.h"


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































