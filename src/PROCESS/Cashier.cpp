#include <unistd.h>
#include <iostream>
#include <signal.h>
#include <string.h>
#include <fstream>
#include <vector>
#include <errno.h>
#include "../../include/Config.h"
#include "../../include/Utils.h"
#include "../../include/Logger.h"
#include "../../include/Signals.h"
#include "../../include/IPC/MessageQueue.h"
#include "../../include/IPC/SharedMemory.h"
#include "../../include/IPC/Semaphore.h"

int main(int argc, char *argv[]) {
	int cashier_id = std::stoi(argv[1]);
	
	SharedData* data = static_cast<SharedData*>(SHAREDMEMORY::attach());
	int semid = SEMAPHORE::getID();
	int mq_register_id = MESSAGEQUEUE::getRegisterID();
	int mq_client_id = MESSAGEQUEUE::getClientMQID();
	
	ReceiptMessage my_msg;
	
	std::ofstream file(RECEIPT_PATH, std::ios::app);
	if(!file.is_open()) {
		std::cerr << "Could not open Receipt file\n";
		return 1;
	}
	
	LOGGER::log("Register " + std::to_string(cashier_id) + " process starts\n");
	
	while(data->is_open) {
		
		if(msgrcv(mq_register_id, &my_msg, sizeof(my_msg) - sizeof(long), cashier_id, 0) == -1) {
			if (errno == EINTR) {
				break;
			}
		
			if(data->is_evacuation) {
				LOGGER::log("Register " + std::to_string(cashier_id) + " is forcefullyy closing\n");
				std::cout << "FORCEFULLY CLOSING REGISTER " << cashier_id << "\n";
				break;
			}
		} else {
			LOGGER::log("Register " + std::to_string(cashier_id) + " serves the customer " + std::to_string(my_msg.client) + "\n");
			
			usleep(350000);
			
			LOGGER::log("Register " + std::to_string(cashier_id) + " starts scanning products " + std::to_string(my_msg.client) + "\n");
			
			if(SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::RECEIPT_MUTEX))) {
				LOGGER::log("Register " + std::to_string(cashier_id) + " locks mutex for " + std::to_string(my_msg.client) + "\n");
			
				float total = 0;
				
				file << "\n==================================\n";
				file << "	RECEIPT - CLIENT " << std::to_string(my_msg.client) << "\n";
				file << "==================================\n";
				
				for(int i = 0; i < PRODUCTS; i++) {
					if(my_msg.counts[i] > 0) {
					
						SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
						data->total_sold[i]++;
						SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
					
						file << Products_base[i].label << " " << std::to_string(my_msg.counts[i]) << " - " << std::to_string(my_msg.counts[i] * Products_base[i].price) << "\n";
						total += my_msg.counts[i] * Products_base[i].price;
					}
				}
				file << "==================================\n";
				file << "TOTAL: " << total << "$\n";
				file << "==================================\n\n";
				file.flush();
				
				SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::RECEIPT_MUTEX));
				
				LOGGER::log("Register sends checkout to " + std::to_string(my_msg.mtype) + "\n");
				
				my_msg.mtype = my_msg.client;
				if(msgsnd(mq_client_id, &my_msg, sizeof(ReceiptMessage) - sizeof(long), 0)) {
					std::cerr << "could not send shopping list to cashier\n";
				}
				
				LOGGER::log("Register sent checkout to " + std::to_string(my_msg.mtype) + " and now is trying to raise signal\n");
				SIGNALS::send(my_msg.client, SIGRTMIN + 1);
				LOGGER::log("Register sent signal to " + std::to_string(my_msg.mtype) + "\n");
				
				
				
				if(data->is_evacuation) {
					LOGGER::log("Register " + std::to_string(cashier_id) + " is preparing to close\n");
					std::cout << "PREPARING TO CLOSE REGISTER " << cashier_id << "\n";
					break;
				}
			}
			LOGGER::log("Register " + std::to_string(cashier_id) + " finished scanning products " + std::to_string(my_msg.client) + "\n");
		}
	}
	
	if(!data->is_evacuation) {
		while (msgrcv(mq_register_id, &my_msg, sizeof(my_msg) - sizeof(long), cashier_id, IPC_NOWAIT) != -1) {
			LOGGER::log("Register " + std::to_string(cashier_id) + " serves the customer " + std::to_string(my_msg.client) + "\n");
						
			usleep(350000);
			
			if(SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::RECEIPT_MUTEX))) {
				LOGGER::log("Register " + std::to_string(cashier_id) + " locks mutex for " + std::to_string(my_msg.client) + "\n");
			
				float total = 0;
				
				file << "\n==================================\n";
				file << "	RECEIPT - CLIENT " << std::to_string(my_msg.client) << "\n";
				file << "==================================\n";
				
				for(int i = 0; i < PRODUCTS; i++) {
					if(my_msg.counts[i] > 0) {
					
						SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
						data->total_sold[i]++;
						SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
					
						file << Products_base[i].label << " " << std::to_string(my_msg.counts[i]) << " - " << std::to_string(my_msg.counts[i] * Products_base[i].price) << "\n";
						total += my_msg.counts[i] * Products_base[i].price;
					}
				}
				file << "==================================\n";
				file << "TOTAL: " << total << "$\n";
				file << "==================================\n\n";
				file.flush();
				
				SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::RECEIPT_MUTEX));
				
				LOGGER::log("Register sends checkout to " + std::to_string(my_msg.mtype) + "\n");
				
				my_msg.mtype = my_msg.client;
				if(msgsnd(mq_client_id, &my_msg, sizeof(ReceiptMessage) - sizeof(long), 0)) {
					std::cerr << "could not send shopping list to cashier\n";
				}
				
				LOGGER::log("Register sent checkout to " + std::to_string(my_msg.mtype) + " and now is trying to raise signal\n");
				SIGNALS::send(my_msg.client, SIGRTMIN + 1);
				LOGGER::log("Register sent signal to " + std::to_string(my_msg.mtype) + "\n");
			}
			LOGGER::log("Register " + std::to_string(cashier_id) + " finished scanning products " + std::to_string(my_msg.client) + "\n");
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
