#ifndef CONFIG_H_
#define CONFIG_H_

struct server_config {
	string address;
	int port;
	int timeout; //in seconds

	server_config() {
	}
	server_config(string addr, int p, int t) :
			address(addr), port(p), timeout(t) {
	}

	string to_string();
};

#endif /* CONFIG_H_ */
