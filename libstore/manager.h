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
    unsigned long lease_duration; 
    
public:
	void init();
    
	static int storage_server_list_initialized;

public:
	static server_manager& instance();

	void init_servers(string);
    void set_lease_duration(unsigned long dur);

	server_config& getServerByHashCode(uint32_t);

};

#endif /* MANAGER_H_ */
