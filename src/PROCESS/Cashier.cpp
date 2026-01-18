#include <unistd.h>
#include <iostream>
#include <signal.h>
#include <string.h>
#include <fstream>
#include <vector>
#include "../../include/Config.h"
#include "../../include/Utils.h"
#include "../../include/Logger.h"
#include "../../include/IPC/MessageQueue.h"
#include "../../include/IPC/SharedMemory.h"
#include "../../include/IPC/Semaphore.h"

bool should_close = false;

static void handlerSigOne(int sig) {
	std::cout << "TEST\n";
	
	return;
}

int main(int argc, char *argv[]) {
	int cashier_id = std::stoi(argv[1]);

	struct sigaction sa;
	sa.sa_handler = handlerSigOne;
	sigaction(SIGUSR2, &sa, NULL);
	
	SharedData* data = static_cast<SharedData*>(SHAREDMEMORY::attach());
	int semid = SEMAPHORE::getID();
	int mq_register_id = MESSAGEQUEUE::getRegisterID();
	
	ReceiptMessage my_msg;
	
	std::ofstream file(RECEIPT_PATH, std::ios::app);
	if(!file.is_open()) {
		std::cerr << "Could not open Receipt file\n";
		return 1;
	}
	
	if(cashier_id > 1) {
		SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
		data->second_register_active = true;
		SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
	}
	std::cout << "CASHIER " << cashier_id << " STARTS RUNNING! :) \n";
	
	while(true) {
		if (cashier_id > 1 && (data->current_customers_count < (MAX_CLIENT_INSIDE / 2))) {
            		should_close = true;
        	}
		
		if(msgrcv(mq_register_id, &my_msg, sizeof(my_msg) - sizeof(long), cashier_id, IPC_NOWAIT) == -1) {
			if(data->is_evacuation) {
				LOGGER::log("Register " + std::to_string(cashier_id) + " is forcefullyy closing\n");
				std::cout << "FORCEFULLY CLOSING REGISTER " << cashier_id << "\n";
				break;
			}
				
				
			usleep(10000);
		} else {
			LOGGER::log("Register " + std::to_string(cashier_id) + " serves the customer " + std::to_string(my_msg.client) + "\n");
			
			// simulate scanning time before mutex
			for(int i = 0; i< PRODUCTS; i++) {
				if(my_msg.counts[i] > 0) 
					usleep(CASHIER_PRODUCT_SCAN_TIME * SIMULATION_MINUTE);
			}
			
			SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::RECEIPT_MUTEX));
			
			float total = 0;
			
			file << "\n==================================\n";
			file << "	RECEIPT - CLIENT " << std::to_string(my_msg.client) << "\n";
			file << "==================================\n";
			
			for(int i = 0; i < PRODUCTS; i++) {
				if(my_msg.counts[i] > 0) {
					file << Products_base[i].label << " " << std::to_string(my_msg.counts[i]) << " - " << std::to_string(my_msg.counts[i] * Products_base[i].price) << "\n";
					total += my_msg.counts[i] * Products_base[i].price;
				}
			}
			file << "==================================\n";
			file << "TOTAL: " << total << "$\n";
			file << "==================================\n\n";
			file.flush();
			
			SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::RECEIPT_MUTEX));
			
			my_msg.mtype = my_msg.client;
			std::cout << "\t" << my_msg.mtype << "\n";
			if(msgsnd(mq_register_id, &my_msg, sizeof(ReceiptMessage) - sizeof(long), 0)) {
				std::cerr << "could not send shopping list to cashier\n";
			}
			
			if(should_close || data->is_evacuation) {
				LOGGER::log("Register " + std::to_string(cashier_id) + " is preparing to close\n");
				std::cout << "PREPARING TO CLOSE REGISTER " << cashier_id << "\n";
				break;
			}
		}
	}
	
	if(!data->is_evacuation) {
		while (msgrcv(mq_register_id, &my_msg, sizeof(my_msg) - sizeof(long), cashier_id, IPC_NOWAIT) != -1) {
			LOGGER::log("Register " + std::to_string(cashier_id) + " serves the customer " + std::to_string(my_msg.client) + "\n");
						
			// simulate scanning time before mutex
			for(int i = 0; i< PRODUCTS; i++) {
				if(my_msg.counts[i] > 0) 
					usleep(CASHIER_PRODUCT_SCAN_TIME * SIMULATION_MINUTE);
			}
			
			SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::RECEIPT_MUTEX));
				
			float total = 0;
				
			file << "\n==================================\n";
			file << "	RECEIPT - CLIENT " << std::to_string(my_msg.client) << "\n";
			file << "==================================\n";
				
			for(int i = 0; i < PRODUCTS; i++) {
				if(my_msg.counts[i] > 0) {
					file << Products_base[i].label << " " << std::to_string(my_msg.counts[i]) << " - " << std::to_string(my_msg.counts[i] * Products_base[i].price) << "\n";
					total += my_msg.counts[i] * Products_base[i].price;
				}
			}
			file << "==================================\n";
			file << "TOTAL: " << total << "$\n";
			file << "==================================\n\n";
			file.flush();
				
			SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::RECEIPT_MUTEX));
			
			my_msg.mtype = my_msg.client;
			std::cout << "\t" << my_msg.mtype << "\n";
			if(msgsnd(mq_register_id, &my_msg, sizeof(ReceiptMessage) - sizeof(long), 0)) {
				std::cerr << "could not send shopping list to cashier\n";
			}
		}
	}
	
	if(cashier_id > 1) {
		std::cout << "CLOSING REGISTER " << cashier_id << "\n";
		SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
		data->second_register_active = false;
		SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
	}
	
	file.close();
	SHAREDMEMORY::detach();
	return 0;
}
