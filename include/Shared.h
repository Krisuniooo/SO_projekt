#ifndef SHARED_H
#define SHARED_H

#include <string>

struct SharedData {
	// flags
	bool is_running; 
	bool is_open;

	int current_customers_count;
	int today_customers_count;
};

class SharedMemManager {
	private:
		int shm_id;
		void* shm_addr;
		bool is_owner;

	public:
		SharedMemManager(const std::string& key_path, char id, size_t size, bool is_owner);
		~SharedMemManager();

		int getSize() const;
		void* getAddress() const;
};

#endif
