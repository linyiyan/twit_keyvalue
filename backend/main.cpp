#include <unordered_map>
#include <vector>
#include <list>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <pthread.h>
#include <string>
#include <sstream>
#include <map>
#include <iterator>

#include <event.h>
//for http
#include <evhttp.h>

pthread_mutex_t lru_lock;

using namespace std;

#include "config.h"
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
    if(method=="setup"){
      oss<<"send you a list of servers"<<endl;
      cout<<"welcome, send you a list"<<endl;
      
      transform(storage_servers.begin() , storage_servers.end() , 
                ostream_iterator<string>(oss , "\n"),
                [](map<string,string>& m){return m["ip"]+":"+m["port"];});  	
    }
    else if(method=="get"){
      // oss<<"Ok, you can get."<<endl;
    	
      string szKey = evhttp_find_header(&params , "key");
      unsigned int key = stol(szKey);
      
      Chunk ck = get(key);
    	
      if(!ck.is_valid()) oss<<"";// oss<<"But data not found."<<endl;
      else // oss<<"Here is the data: "<<ck.get_data()<<endl;
        oss<<ck.get_data()<<" ";
    }
    else if(method=="set"){      
      // oss<<"Ok, you can set";
      
      string szKey = evhttp_find_header(&params , "key");
      unsigned int key = stol(szKey);
        
      string szVal = evhttp_find_header(&params , "value");
    	
      set(key , const_cast<char*>(szVal.c_str()) , szVal.size());
     
    }
    else if(method=="incr"){
      // oss<<"Ok, you can incr";
      
      string szKey = evhttp_find_header(&params , "key");
      unsigned int key = stol(szKey);
      
      Chunk ck = get(key);
      if(!ck.is_valid()) {
        // oss<<"incr data not found. set a new one"<<endl;
        
        string szKey = evhttp_find_header(&params , "key");
        unsigned int key = stol(szKey);

        string szVal = "1";
        set(key , const_cast<char*>(szVal.c_str()) , szVal.size());
       
      } else{
        string szData = ck.get_data();
        auto pos = find_if(szData.begin() , szData.end() , [](char& ch){return !isdigit(ch);});
        
        if(pos!=szData.end()){
          // oss<<"data is not an integer"<<endl;
        } else{
          unsigned int iData = stol(szData);
          szData = to_string(iData+1);
          replace(key , const_cast<char*>(szData.c_str()) , szData.size());
               
        }
        
      }

    } else{
    	oss<<"Oops, method not supported";    	
    }
    
    evhttp_add_header(req->output_headers, "Server", "0.0.0.0");
  	evhttp_add_header(req->output_headers, "Content-Type", "text/plain; charset=UTF-8");
  	evhttp_add_header(req->output_headers, "Connection", "close");
  	
  	
  	struct evbuffer *buf;
  	buf = evbuffer_new();
  	evbuffer_add_printf(buf, "%s%c", oss.str().c_str() , '\0');
 	evhttp_send_reply(req, HTTP_OK, "OK", buf);
  	evbuffer_free(buf);
    
}


int main(int argc, char **argv)
{
	pthread_mutex_init(&lru_lock, NULL); 

    slab_config config("setting");
	// slabs_init(slabs);
    slabs_init(config , slabs);

	init_slab_lru(config);
	
	unsigned short 	port = 0;
	if(argc<2){
		cout<<"specify a port"<<endl;
		return 0;
	} else if(argc<4){
		cout<<"specify a port range"<<endl;
		return 0;
	} else{
		string szPort = argv[1];
		port = stoi(szPort);
		
		string szLower = argv[2]; // lower bound of ports
		string szUpper = argv[3]; // uppper bound of ports
		
		unsigned short pstart = stoi(szLower), pend = stoi(szUpper);
		for(unsigned short p=pstart ; p<=pend ; p++){
			map<string,string> m;
			m["ip"]="0.0.0.0";
			m["port"]=to_string(p);
			storage_servers.push_back(m);
		}
	}

	event_init();
	evhttp *httpd = evhttp_start("0.0.0.0", port);
                   
    evhttp_set_timeout(httpd, 120);
	evhttp_set_gencb(httpd, libstore_http_req_handler, NULL);
	
	event_dispatch();
  
  	evhttp_free(httpd);  	
  		
	return 0;
}























