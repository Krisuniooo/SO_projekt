#include "../include/Logger.h"
#include "../include/Config.h"
#include "../include/Utils.h"
#include "../include/IPC/MessageQueue.h"
#include "../include/IPC/Semaphore.h"

#include <iostream>
#include <string>
#include <fstream>
#include <cstring>
#include <pthread.h>
#include <unistd.h>

static bool logger_thread_end;
static pthread_t thread_id;

bool LOGGER::init() {
	bool setupSuccess = UTILS::setupKeyFile(LOGGER_PATH);
	
	if(setupSuccess == false) {
		std::cerr << "Could not create key file\n";
		return false;
	}
	
	if (pthread_create(&thread_id, nullptr, LOGGER::logThread, nullptr) != 0) {
		std::cerr << "Thread creation failed";
		return false;
	}
	
	logger_thread_end = false;
	
	return true;
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
	
	while(!logger_thread_end) {
		if (msgrcv(mq_logger_id, &msg, sizeof(msg.text), 0, 0) != -1) {
			UTILS::getTimestamp(ts, sizeof(ts));
			file << "[" << ts << "] " << msg.text;
			file.flush();
		}
	}
	
	// read rest of the mq
	while (msgrcv(mq_logger_id, &msg, sizeof(msg.text), 0, IPC_NOWAIT) != -1) {
		UTILS::getTimestamp(ts, sizeof(ts));
		file << "[" << ts << "] " << msg.text;
	}
	
	UTILS::getTimestamp(ts, sizeof(ts));
	file << "[" << ts << "] " << "Logger stopped\n";
	file.flush();
	
	file.close();
	
	return nullptr;
}

void LOGGER::endLogThread() { 
	logger_thread_end = true; 
	LOGGER::log("\n"); // empty message to wake up msgrcv
	pthread_join(thread_id, nullptr);
}
