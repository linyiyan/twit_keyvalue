#ifndef STORAGE_SERVER_MANAGER
#define STORAGE_SERVER_MANAGER



class storage_server_manager{
private: 
	storage_server_manager(){
      storage_servers = vector<server_config>(10 , server_config());
    }
	storage_server_manager(const storage_server_manager&){}
	storage_server_manager& operator= (const storage_server_manager&){}
		
private:
	vector<server_config> storage_servers;
	
		
public: 
	static storage_server_manager& instance(){
		static storage_server_manager inst;
		return inst;	
	}

    server_config& getServerByHashCode(uint32_t);

    
	
};

#endif
