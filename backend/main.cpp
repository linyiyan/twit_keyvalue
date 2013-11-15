#include <unordered_map>
#include <vector>
#include <list>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <pthread.h>
#include <string>
#include <sstream>
#include <fstream>
#include <map>
#include <iterator>
#include <event.h>
#include <evhttp.h>

pthread_mutex_t lru_lock;

using namespace std;

#include "config.h"
#include "chunk.h"
#include "slab.h"
#include "lrulist.h"
#include "global.h"
#include "handlers.h"

extern vector<map<string,string>> storage_servers;
extern vector<Slab> slabs;


int main(int argc, char **argv)
{
	pthread_mutex_init(&lru_lock, NULL); 

    slab_config config("setting");
	// slabs_init(slabs);
    slabs_init(config , slabs);

	init_slab_lru(config);
	
	unsigned short 	port = 0;
	if(argc<2){
		cout<<"specify a server address file"<<endl;
		return 0;
	} else if(argc<3){
		cout<<"specify a port"<<endl;
	}
	else{
		ifstream in(argv[1]);
		while(!in.eof()){
			string szIp = "", szPort = "";
			in>>szIp>>szPort;

			if(szIp=="" || szPort=="") continue;

			map<string,string> m;
			m["ip"] = szIp;
			m["port"] = szPort;

			storage_servers.push_back(m);
		}

		string szPort = argv[2];
		port = stoi(szPort);
	}

	event_init();
	evhttp *httpd = evhttp_start("0.0.0.0", port);
                   
    evhttp_set_timeout(httpd, 120);
	evhttp_set_gencb(httpd, libstore_http_req_handler, NULL);
	
	event_dispatch();
  
  	evhttp_free(httpd);  	
  		
	return 0;
}























