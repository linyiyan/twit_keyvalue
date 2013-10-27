#include <stdint.h>
#include <limits.h>
#include <vector>
#include <string>
#include <iostream>


using std::string;
using std::vector;
using std::cout;
using std::endl;





#include "config.h"
#include "storage_server_manager.h"


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
