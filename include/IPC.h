#ifndef IPC_H
#define IPC_H

#include "IPC/SharedMemory.h"
#include "IPC/Semaphore.h"
#include "IPC/MessageQueue.h"
#include "IPC/Fifo.h"

namespace IPC {
	bool init();
	
	bool destroyAll();
}

#endif
