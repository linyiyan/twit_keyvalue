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

using namespace std;

#include "twit_hash.h"
#include "config.h"
#include "storage_server_manager.h"
#include "handlers.h"




int main(int argc, char *argv[]) {
    
  server_config twit_server_config("0.0.0.0" , 8080 , 120);

  storage_server_manager& mgr = storage_server_manager::instance();
  mgr.init();
 
 /*
  set(mgr , "t3" , "test1");
  set(mgr , "t2" , "test2");
  
  get(mgr,"t2");
  get(mgr,"t3");
  */
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
