#include "../../include/IPC/MessageQueue.h"
#include "../../include/Config.h"
#include "../../include/Utils.h"

#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <cerrno>
#include <fstream>

static int mq_logger_id = -1;

bool MESSAGEQUEUE::init() {
	bool setupSuccess = UTILS::setupKeyFile(MESSAGE_QUEUE_KEY_PATH);
	
	if(setupSuccess == false) {
		std::cerr << "Could not create key file\n";
		return false;
	}
	
	return MESSAGEQUEUE::createLoggerMQ();
}

bool MESSAGEQUEUE::createLoggerMQ() { 
	mq_logger_id = MESSAGEQUEUE::getLoggerID(IPC_CREAT | IPC_EXCL | 0600);
	
	if(mq_logger_id == -1) return false;
	
	return true;
}

int MESSAGEQUEUE::getLoggerID(int flags) {
	if(mq_logger_id != -1) return mq_logger_id;

	key_t key = ftok(MESSAGE_QUEUE_KEY_PATH, MESSAGE_QUEUE_KEY);

	if(key == -1) {
		std::cerr << "ftok Error: " << strerror(errno) << "\n";
		return -1;
	}
	
	mq_logger_id = msgget(key, flags);
	
	if(mq_logger_id == -1) {
		std::cerr << "semget Error: " << strerror(errno) << "\n";
	}
	
	return mq_logger_id;
}
