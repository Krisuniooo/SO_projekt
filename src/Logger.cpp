#include "../include/Logger.h"
#include "../include/Config.h"
#include "../include/Utils.h"
#include "../include/IPC/MessageQueue.h"
#include "../include/IPC/Semaphore.h"

#include <iostream>
#include <string>
#include <fstream>
#include <cstring>

static bool logger_thread_run = false;

bool LOGGER::init() {
	bool setupSuccess = UTILS::setupKeyFile(LOGGER_PATH);
	
	return setupSuccess;
}

void LOGGER::log(const std::string& message) {
	int mq_logger_id = MESSAGEQUEUE::getLoggerID();
	if(mq_logger_id == -1) {
		std::cerr << "Log message Queue not initialized\n";
		return;
	}
	
	LogMessage msg;
	msg.mtype = 1;
	strncpy(msg.text, message.c_str(), sizeof(msg.text) - 1);
	
	if (msgsnd(mq_logger_id, &msg, sizeof(msg.text), IPC_NOWAIT) == -1) {
		if (errno == EAGAIN) {
			std::cerr << "Log message overflow\n";
		} else {
			std::cerr << "Log message Error\n";
		}
	}
	
}

void* LOGGER::logThread(void* arg) {
	int mq_logger_id = MESSAGEQUEUE::getLoggerID();
	if(mq_logger_id == -1) {
		std::cerr << "Log message Queue not initialized\n";
		return nullptr;
	}

	std::ofstream file(LOGGER_PATH, std::ios::app);
	
	if(!file.is_open()) {
		std::cerr << "Could not open Logger file for Logger thread\n";
		return nullptr;
	}
	
	LogMessage msg;
	char ts[32];
	logger_thread_run = true;
	
	while(logger_thread_run) {
		if (msgrcv(mq_logger_id, &msg, sizeof(msg.text), 0, 0) != -1) {
			UTILS::getTimestamp(ts, sizeof(ts));
			file << "[" << ts << "] " << msg.text;
			file.flush();
		}
	}
	
	file.close();
	
	SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::LOGGER_SEM_MUTEX));
	
	return nullptr;
}

void LOGGER::endLogThread() { 
	LOGGER::log("\n");
	logger_thread_run = false; 
}
