#include "../../include/IPC/MessageQueue.h"
#include "../../include/Config.h"

#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <cerrno>
#include <fstream>

static int mq_testmq_id = -1;

bool MESSAGEQUEUE::init() {
	bool setupSuccess = MESSAGEQUEUE::setupKeyFile();
	
	if(setupSuccess == false) {
		std::cerr << "Could not create key file\n";
		return false;
	}
	
	return MESSAGEQUEUE::create();
}

bool MESSAGEQUEUE::create() { 
	mq_testmq_id = MESSAGEQUEUE::getID(IPC_CREAT | IPC_EXCL | 0600);
	
	if(mq_testmq_id == -1) return false;
	
	return true;
}

int MESSAGEQUEUE::getID(int flags) {
	if(mq_testmq_id != -1) return mq_testmq_id;

	key_t key = ftok(MESSAGE_QUEUE_KEY_PATH, MESSAGE_QUEUE_KEY);

	if(key == -1) {
		std::cerr << "ftok Error: " << strerror(errno) << "\n";
		return -1;
	}
	
	mq_testmq_id = msgget(key, flags);
	
	if(mq_testmq_id == -1) {
		std::cerr << "semget Error: " << strerror(errno) << "\n";
	}
	
	return mq_testmq_id;
}


bool MESSAGEQUEUE::setupKeyFile() {
	std::ofstream file(MESSAGE_QUEUE_KEY_PATH, std::ios::app);
	
	if(!file.is_open()) 
		return false;
		
	file.close();
	return true;
}
