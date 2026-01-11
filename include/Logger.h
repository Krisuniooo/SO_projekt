#ifndef LOGGER_H
#define LOGGER_H

#include <string>

namespace LOGGER {
	bool init();
	
	void log(const std::string& message);
	void* logThread(void* arg);
	void endLogThread();
}

#endif
