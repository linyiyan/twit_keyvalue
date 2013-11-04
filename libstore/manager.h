#ifndef MANAGER_H_
#define MANAGER_H_

class server_manager {
private:
	server_manager();
	server_manager(const server_manager&) {
	}
	server_manager& operator=(const server_manager&) {
	}

private:
	vector<server_config> storage_servers;

public:
	void init();
	static int storage_server_list_initialized;

public:
	static server_manager& instance();

	void init_servers(string);

	server_config& getServerByHashCode(uint32_t);

};

#endif /* MANAGER_H_ */
