#include <string>
#include <sstream>

using std::string;
using std::ostringstream;

#include "config.h"

string server_config::to_string() {
	ostringstream oss;
	oss << address << ":" << port;
	return oss.str();
}
