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
		SharedMemManager(const std::string& key_path, char id, size_t size, bool is_owner = false);
		~SharedMemManager();

		int getSize() const;
		int getID() const;
		void* getAddress() const;
};

class Semaphore {
	private:
		int sem_id;
	
	public:
		Semaphore(const std::string& key_path, char id, int init_val = 1);
		~Semaphore();
		
		void waitSem(int sem_num);
		void signalSem(int sem_num);
		void writeSem(int sem_num, int new_val);
		int readSem(int sem_num);
	
};

bool setupKeyFile(const std::string& key_path);

#endif
