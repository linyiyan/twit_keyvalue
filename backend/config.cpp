#include <string>
#include <fstream>
#include <vector>
#include <map>
#include <string>

using std::string;
using std::ifstream;
using std::vector;
using std::pair;
using std::stoi;
using std::make_pair;

#include "config.h"


slab_config::slab_config(string file_name){

  ifstream in(file_name);
  
  string line;
  while(in>>line){
    auto pos = line.find(",");
    slab_settings.push_back(make_pair(stoi(line.substr(0,pos)) , stoi(line.substr(pos+1))));
  }
    
  in.close();
}
