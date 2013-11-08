#ifndef CONFIG_H
#define CONFIG_H

class slab_config {

private:
	slab_config() {
	}

public:
	vector<pair<int, int>> slab_settings;

public:
	slab_config(string file_name);

};

#endif
