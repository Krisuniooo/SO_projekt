#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>

namespace MESSAGEQUEUE {
	bool init();
	
	bool createClientMQ();
	int getClientMQID(int flags = 0600);
	
	bool destroyClientMQ();
	
	bool createRegisterMQ();
	int getRegisterID(int flags = 0600);
	
	bool destroyRegister();
}

#endif
