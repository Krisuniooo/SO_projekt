#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>

namespace MESSAGEQUEUE {
	bool init();
	
	bool create();
	int getID(int flags = 0600);
	bool setupKeyFile();
}

#endif
