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


using namespace std;

#include "twit_hash.h"


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
