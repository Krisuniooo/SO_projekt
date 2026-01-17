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
static int mq_register_id = -1;

bool MESSAGEQUEUE::init() {
	bool setupSuccess = UTILS::setupKeyFile(MESSAGE_QUEUE_KEY_PATH);
	
	if(setupSuccess == false) {
		std::cerr << "Could not create key file\n";
		return false;
	}
	
	return (MESSAGEQUEUE::createLoggerMQ() && MESSAGEQUEUE::createRegisterMQ());
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

bool MESSAGEQUEUE::destroyLogger() {
	if(mq_logger_id == -1) {
		std::cerr << "msgctl error (IPC_RMID): Message queue not initialized\n";
		return false;
	} 
	
	if(msgctl(mq_logger_id, IPC_RMID, 0) == -1) {
		std::cerr << "msgctl error (IPC_RMID): " << strerror(errno) << "\n";
		return false;
	}
	mq_logger_id = -1;
	return true;
}

bool MESSAGEQUEUE::createRegisterMQ() { 
	mq_register_id = MESSAGEQUEUE::getRegisterID(IPC_CREAT | IPC_EXCL | 0600);
	
	if(mq_register_id == -1) return false;
	
	return true;
}

int MESSAGEQUEUE::getRegisterID(int flags) {
	if(mq_register_id != -1) return mq_register_id;

	key_t key = ftok(MESSAGE_QUEUE_KEY_PATH, MESSAGE_QUEUE_REGISTER_KEY);

	if(key == -1) {
		std::cerr << "ftok Error: " << strerror(errno) << "\n";
		return -1;
	}
	
	mq_register_id = msgget(key, flags);
	
	if(mq_register_id == -1) {
		std::cerr << "semget Error: " << strerror(errno) << "\n";
	}
	
	return mq_register_id;
}

bool MESSAGEQUEUE::destroyRegister() {
	if(mq_register_id == -1) {
		std::cerr << "msgctl error (IPC_RMID): Message queue not initialized\n";
		return false;
	} 
	
	if(msgctl(mq_register_id, IPC_RMID, 0) == -1) {
		std::cerr << "msgctl error (IPC_RMID): " << strerror(errno) << "\n";
		return false;
	}
	mq_register_id = -1;
	return true;
}
