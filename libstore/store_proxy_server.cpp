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

using namespace std;

#include "twit_hash.h"
#include "handlers.h"
#include "config.h"
#include "storage_server_manager.h"




int main(int argc, char *argv[]) {
    
  server_config twit_server_config("0.0.0.0" , 8080 , 120);

  storage_server_manager& mgr = storage_server_manager::instance();
  
 
  
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
