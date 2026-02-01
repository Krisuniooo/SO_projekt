#include "../../include/IPC/MessageQueue.h"
#include "../../include/Config.h"
#include "../../include/Utils.h"

static int mq_client_id = -1;
static int mq_register_id = -1;

bool MESSAGEQUEUE::init() {
	bool setupSuccess = UTILS::setupKeyFile(MESSAGE_QUEUE_KEY_PATH);
	
	if(setupSuccess == false) {
		perror("Could not create key file");
		return false;
	}
	
	return (MESSAGEQUEUE::createClientMQ() && MESSAGEQUEUE::createRegisterMQ());
}

bool MESSAGEQUEUE::createClientMQ() { 
	mq_client_id = MESSAGEQUEUE::getClientMQID(IPC_CREAT | IPC_EXCL | 0600);
	
	if(mq_client_id == -1) return false;
	
	return true;
}

int MESSAGEQUEUE::getClientMQID(int flags) {
	if(mq_client_id != -1) return mq_client_id;

	key_t key = ftok(MESSAGE_QUEUE_KEY_PATH, MESSAGE_QUEUE_KEY);

	if(key == -1) {
		perror("ftok Error");
		return -1;
	}
	
	mq_client_id = msgget(key, flags);
	
	if(mq_client_id == -1) {
		perror("msgget Error");
	}
	
	return mq_client_id;
}

bool MESSAGEQUEUE::destroyClientMQ() {
	if(mq_client_id == -1) {
		perror("msgctl Error (IPC_RMID): Messege queue not initialized");
		return false;
	} 
	
	if(msgctl(mq_client_id, IPC_RMID, 0) == -1) {
		perror("msgctl Error (IPC_RMID)");
		return false;
	}
	mq_client_id = -1;
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
