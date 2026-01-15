#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <signal.h>

#include "../include/IPC.h"
#include "../include/Logger.h"
#include "../include/Config.h"

bool checkConfig() {
	if((static_cast<int>(SemaphoreTypes::PRODUCTS_BASE_END) - static_cast<int>(SemaphoreTypes::PRODUCTS_BASE) - 1) != PRODUCTS) {
		std::cerr << "Config error: N doesnt match semaphore count";
		return false;
	}
	
	return true;
}

void generateBaker() {
	pid_t pid = fork();
	if(pid == -1) {
    		std::cerr << "IPC initialization error \n";
    		exit(1);
    	} else if(pid == 0) {
    		std::cout << "Generating Baker\n";
    		execl("./baker", "baker", NULL);
    		std::cerr << "Baker process could not be created! \n";
    		exit(1);
    	}
}

int main() {
	if (!checkConfig()) {
	    	std::cerr << "Config error \n";
    		return 1;
	}

    	if (!IPC::init()) {
    		std::cerr << "IPC initialization error \n";
    		return 1;
    	}
    	
	if (!LOGGER::init()) {
    		std::cerr << "LOGGER initialization error \n";
    		return 1;
    	}
    	LOGGER::log("Initialization successful\n");
	
	generateBaker();
	
	LOGGER::endLogThread();
	
	bool destroy_success = IPC::destroyAll();
	if(!destroy_success) {
		std::cerr << "could not destroy IPC\n";
		return 1;
	}

	return 0;
}
