#ifndef IPC_H
#define IPC_H

#include "IPC/SharedMemory.h"
#include "IPC/Semaphore.h"
#include "IPC/MessageQueue.h"

namespace IPC {
	bool init();
}

#endif
