#ifndef STORAGE_SERVER_MANAGER
#define STORAGE_SERVER_MANAGER



class storage_server_manager{
private: 
	storage_server_manager();
	storage_server_manager(const storage_server_manager&){}
	storage_server_manager& operator= (const storage_server_manager&){}
		
private:
	vector<server_config> storage_servers;
	
	
public:
	void init();
	static int storage_server_list_initialized;
		
public: 
	static storage_server_manager& instance();

	void init_servers(string );
	
    server_config& getServerByHashCode(uint32_t);
    
    

    
	
};

#endif
