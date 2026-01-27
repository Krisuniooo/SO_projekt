#include "../include/Logger.h"
#include "../include/Config.h"
#include "../include/Utils.h"
#include "../include/IPC/MessageQueue.h"
#include "../include/IPC/Semaphore.h"

#include <iostream>
#include <string>
#include <pthread.h>
#include <unistd.h>

bool LOGGER::init() {
	bool setupSuccess = UTILS::setupKeyFile(LOGGER_PATH);
	
	if(setupSuccess == false) {
		std::cerr << "Could not create key file\n";
		return false;
	}
	
	return true;
}

void LOGGER::log(const std::string& message) {
	if(SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::LOGGER_MUTEX))) {
		FILE* file = fopen(LOGGER_PATH, "a");
		
		if(!file) {
			std::cerr << "Could not save message - data might be lost\n";
			SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::LOGGER_MUTEX));
			return;
		}
		
		char ts[32];
		UTILS::getTimestamp(ts, sizeof(ts));
		fprintf(file, "[%s] %s", ts, message.c_str());
		
		fclose(file);
		
		SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::LOGGER_MUTEX));
	}
}


