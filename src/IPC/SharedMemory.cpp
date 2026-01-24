#include "../../include/IPC/SharedMemory.h"
#include "../../include/Config.h"
#include "../../include/Utils.h"

#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <cerrno>
#include <fstream>

static int shm_id = -1;
static void* shm_ptr = nullptr;

// THIS SHOULD BE USED ONLY IN MAIN
bool SHAREDMEMORY::init() {
	bool setupSuccess = UTILS::setupKeyFile(SHARED_MEM_KEY_PATH);
	
	if(setupSuccess == false) {
		std::cerr << "Could not create key file\n";
		return false;
	}
	
	bool createSuccess = SHAREDMEMORY::create();
	
	if(createSuccess == false) {
		std::cerr << "Could not create shared memory block\n";
		return false;
	}
	
	SharedData* data = static_cast<SharedData*>(SHAREDMEMORY::attach());
	if (data == (void*)-1) {
		std::cerr << "Attach failed\n";
		return false;
	}
	
	data->is_running = false;
	data->is_open = false;
	data->is_stocktaking = false;
	data->is_evacuation = false;
	data->current_customers_count = 0;
	data->today_customers_count = 0;
	data->second_register_active = false;
	
	for(int i = 0; i<PRODUCTS; i++) {
		Tray newTray;
		newTray.head = 0;
		newTray.tail = 0;
		newTray.count = 0;
		
		data->trays[i] = newTray;
	}
	
	return true;
}

bool SHAREDMEMORY::create() {
	int block_id = SHAREDMEMORY::getID(IPC_CREAT | IPC_EXCL | 0600);
	
	if(block_id == -1) {
		std::cerr << "Shared Memory IPC_CREAT Error\n";
		return false;
	}
	
	return true;
}

// Default flags: 0600
int SHAREDMEMORY::getID(int flags) {
	if(shm_id != -1) return shm_id;

	key_t key = ftok(SHARED_MEM_KEY_PATH, SHARED_MEM_KEY);

	if(key == -1) {
		std::cerr << "ftok Error: " << strerror(errno) << "\n";
		return -1;
	}
	
	shm_id = shmget(key, sizeof(SharedData), flags);
	
	if(shm_id == -1) {
		std::cerr << "shmget Error: " << strerror(errno) << "\n";
	}
	
	return shm_id;
}

void* SHAREDMEMORY::attach() {
	if(shm_ptr != nullptr) return shm_ptr;
	if(shm_id == -1 && SHAREDMEMORY::getID() == -1) return nullptr;
	shm_ptr = shmat(shm_id, nullptr, 0);

	if(shm_ptr == (void*)-1) {
		std::cerr << "shmat Error: " << strerror(errno) << "\n";
		shm_ptr = nullptr;
	}
	return shm_ptr;
}

bool SHAREDMEMORY::detach() {
	if(shm_ptr == nullptr || shm_ptr == (void*)-1) return true;

	if(shmdt(shm_ptr) == -1) {
		std::cerr << "shmdt Error: " << strerror(errno) << "\n";
		return false;
	}
	
	shm_ptr = nullptr;
	return true;
}

bool SHAREDMEMORY::destroy() {
	if(shm_id == -1) return false;
	
	if(shmctl(shm_id, IPC_RMID, nullptr) == -1) {
		std::cerr << "shmctl Error (delete): " << strerror(errno) << "\n";
		return false;
	}
	shm_id = -1;
	return true;
}
