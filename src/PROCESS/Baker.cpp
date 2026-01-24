#include <unistd.h>
#include <iostream>
#include <signal.h>
#include <string.h>
#include "../../include/Config.h"
#include "../../include/Utils.h"
#include "../../include/Logger.h"
#include "../../include/IPC/SharedMemory.h"
#include "../../include/IPC/Semaphore.h"
#include "../../include/IPC/Fifo.h"

#include <cstdio>
#include <cmath>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
 #include <fcntl.h>
 #include <stdio.h>
 #include <vector>
#include <pthread.h>

static int trays_write_fd[PRODUCTS];
static pthread_t trays_thread_ids[PRODUCTS];

void* bakeProductOnTray(void* arg) {
	int id_product = *static_cast<int*>(arg);
	delete static_cast<int*>(arg);
	
	std::string path = FIFO_PATH + std::to_string(id_product);
	int product_size_bytes = PIPE_BUF / Products_base[id_product].max_stock;
	char data[product_size_bytes];
	memset(data, 'v', product_size_bytes);
	
	signal(SIGPIPE, SIG_IGN);
	
	std::cout << "Opening file " << path.c_str() << "\n";
	int fd = open(path.c_str(), O_WRONLY);

	while(true) {
	
		int write_bytes = write(fd, data, product_size_bytes);
		
		if(write_bytes > 0) {
			std::cout << "Zapisano: " << write_bytes << "\n";
			std::string log_message = "Baker: added " + std::to_string(write_bytes) + " bytes to " + Products_base[id_product].label + ", added " + std::to_string(floor(write_bytes / product_size_bytes)) + " products to conveyor\n";
			LOGGER::log(log_message);
		} else {
			std::cout << "Nie udalo sie zapisac\n";
		}
		

		
		int time = UTILS::getRandom(BAKE_MIN_TIME, BAKE_MAX_TIME) * SIMULATION_MINUTE;
		usleep(static_cast<int>(time));
	}
	
	std::cout << "Closing file " << path.c_str() << "\n";
	close(fd);
	
	
	
	return nullptr;
}

static void handlerSigOne(int sig) {
	std::cout << "TEST\n";
	
	return;
}

int main() {
	struct sigaction sa;
	sa.sa_handler = handlerSigOne;
	sigaction(SIGUSR2, &sa, NULL);
	
	SharedData* data = static_cast<SharedData*>(SHAREDMEMORY::attach());
	int semid = SEMAPHORE::getID();

	for(int i=0; i< PRODUCTS; i++) {
		int* id = new int(i);
		pthread_t tid;
		pthread_create(&tid, NULL, bakeProductOnTray, id);
		trays_thread_ids[i] = tid;
	}

	while(true) {
		int time = UTILS::getRandom(BAKE_MIN_TIME, BAKE_MAX_TIME) * SIMULATION_MINUTE;
		usleep(static_cast<int>(time/10));
		
		if(data->is_evacuation)
			break;
	}
	
	
	SHAREDMEMORY::detach();
	for(int i=0; i<PRODUCTS; i++) {
		close(trays_write_fd[i]);
	}
	
	return 0;
}
