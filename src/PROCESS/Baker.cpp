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

static int trays_write_fd[PRODUCTS];

static void handlerSigOne(int sig) {
	std::cout << "TEST\n";
	
	return;
}

// ten nonblock cos monci
bool openWriteFD() {
	for(int i=0; i<PRODUCTS; i++) {
		std::string path = FIFO_PATH + std::to_string(i);
		int fd = open(path.c_str(), O_WRONLY);
		if(fd == -1) return false;
		
		fcntl(fd, F_SETPIPE_SZ, PIPE_BUF); // RESIZE FIFO TO PIPE_BUF
		trays_write_fd[i] = fd;
	}
	return true;
}

int main() {
	struct sigaction sa;
	sa.sa_handler = handlerSigOne;
	sigaction(SIGUSR2, &sa, NULL);
	
	SharedData* data = static_cast<SharedData*>(SHAREDMEMORY::attach());
	int semid = SEMAPHORE::getID();
	
	sleep(1);
	
	if(openWriteFD() == false) {
		std::cout << "Could not initialize Write file descriptors\n";
		return 1;
	}


	while(true) {
		int time = UTILS::getRandom(BAKE_MIN_TIME, BAKE_MAX_TIME) * SIMULATION_MINUTE;
		usleep(static_cast<int>(time/10));
		
		if(data->is_evacuation)
			break;
			
		for(int i = 0; i<PRODUCTS; i++) {
			int amount = UTILS::getRandom(BAKE_MIN_PRODUCTS, BAKE_MAX_PRODUCTS);
			int product_size_bytes = PIPE_BUF / Products_base[i].max_stock;
			
			char data[product_size_bytes];
			memset(data, 'v', product_size_bytes);
		
			int write_bytes = 0;
			for(int j=0; j<amount; j++) {
				int sz = 0;
				ioctl(trays_write_fd[i], FIONREAD, &sz);
				
				if(sz+product_size_bytes > fcntl(trays_write_fd[i], F_GETPIPE_SZ)) break;
				
				write_bytes += write(trays_write_fd[i], data, product_size_bytes);
			}
			
			int sz = 0;
			ioctl(trays_write_fd[i], FIONREAD, &sz);
			std::cout << i << "\t" << fcntl( trays_write_fd[i], F_GETPIPE_SZ ) << "\t" << write_bytes << "\t" << sz << "\n";
			
			if(write_bytes == -1) continue;
			if(write_bytes < product_size_bytes) continue; // not even one was added
			
			std::string log_message = "Baker: added " + std::to_string(write_bytes) + " bytes to " + Products_base[i].label + ", added " + std::to_string(floor(write_bytes / product_size_bytes)) + " products to conveyor\n";
			LOGGER::log(log_message);
			
			/*if(SEMAPHORE::lock(data->trays[i].sem_num)) {
				if(SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX))) {
				
					if((data->trays[i].in_stock + amount) > Products_base[i].max_stock) {
						//log_message = "Baker: added " + std::to_string(Products_base[i].max_stock - data->trays[i].in_stock) + ", trashed " + std::to_string(data->trays[i].in_stock + amount - Products_base[i].max_stock) +  " " + Products_base[i].label + ", new value: " + std::to_string(data->trays[i].in_stock + amount) + "\n";
						
						#if DEBUG_MESSAGES == 1
							std::cout << log_message;
						#endif
										
						data->trays[i].in_stock = Products_base[i].max_stock;
						std::cout << data->trays[i].in_stock << "\n";
					} else {
						//log_message = "Baker: added " + std::to_string(amount) + " " + Products_base[i].label + ", new value: " + std::to_string(data->trays[i].in_stock + amount) + "\n";
						
						#if DEBUG_MESSAGES == 1
							std::cout << log_message;
						#endif
						
						
						data->trays[i].in_stock = data->trays[i].in_stock + amount;
					}
					
					SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
					SEMAPHORE::unlock(data->trays[i].sem_num);
					//LOGGER::log(log_message);
				}
			}*/
		}
	}
	
	std::cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n";
	
	SHAREDMEMORY::detach();
	for(int i=0; i<PRODUCTS; i++) {
		close(trays_write_fd[i]);
	}
	
	return 0;
}
