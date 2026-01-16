#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <signal.h>
#include <vector>

#include "../include/IPC.h"
#include "../include/Logger.h"
#include "../include/Config.h"

static pthread_t client_gen_thread;
std::vector<pid_t> active_pids;
pthread_mutex_t pid_mutex = PTHREAD_MUTEX_INITIALIZER;
static pid_t pid_baker = -1;
static bool keep_generating = true;

bool checkConfig() {
	if((static_cast<int>(SemaphoreTypes::PRODUCTS_BASE_END) - static_cast<int>(SemaphoreTypes::PRODUCTS_BASE) - 1) != PRODUCTS) {
		std::cerr << "Config error: N doesnt match semaphore count";
		return false;
	}
	
	return true;
}

void add_pid(pid_t pid) {
	pthread_mutex_lock(&pid_mutex);
	active_pids.push_back(pid);
	pthread_mutex_unlock(&pid_mutex);
}

void cleanup_zombies() {
	pthread_mutex_lock(&pid_mutex);
	for(auto it = active_pids.begin(); it != active_pids.end(); ) {
		if (waitpid(*it, NULL, WNOHANG) > 0) {
			it = active_pids.erase(it);
		} else {
			++it;
		}
	}
	pthread_mutex_unlock(&pid_mutex);
}

void generateBaker() {
	pid_baker = fork();
	if(pid_baker == -1) {
    		std::cerr << "IPC initialization error \n";
    		exit(1);
    	} else if(pid_baker == 0) {
    		std::cout << "Generating Baker\n";
    		execl("./baker", "baker", NULL);
    		std::cerr << "Baker process could not be created! \n";
    		exit(1);
    	} 
}

void generateClient() {
	pid_t pid = fork();
	if(pid == -1) {
    		std::cerr << "IPC initialization error \n";
    		exit(1);
    	} else if(pid == 0) {
    		std::cout << "Generating Client\n";
    		execl("./client", "client", NULL);
    		std::cerr << "Client process could not be created! \n";
    		exit(1);
    	} else {
    		add_pid(pid);
    	}
}

void* clientGeneratorRoutine(void* arg) {
	while (keep_generating) {
		generateClient();
		sleep(1); 
		cleanup_zombies();
	}
	return nullptr;
}

void handleSigInt(int sig) {
	keep_generating = false;
	pthread_cancel(client_gen_thread);
	
	for(pid_t p : active_pids) {
		kill(p, SIGTERM); 
	}
	
	for(pid_t p : active_pids) {
		waitpid(p, NULL, 0);
	}
	
	if(pid_baker != -1) 
		kill(pid_baker, SIGTERM);
	
	LOGGER::endLogThread();
	SHAREDMEMORY::detach();
	
	bool destroy_success = IPC::destroyAll();
	if(!destroy_success) {
		std::cerr << "could not destroy IPC\n";
	}
}

int main() {
	struct sigaction sa;
	sa.sa_handler = handleSigInt;
	sigaction(SIGINT, &sa, NULL);

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
    	
    	SharedData* data = static_cast<SharedData*>(SHAREDMEMORY::attach());
    	data->is_running = true;
	
	generateBaker();
	
	if(pthread_create(&client_gen_thread, NULL, clientGeneratorRoutine, NULL) != 0) {
		std::cerr << "Failed to create client generator thread\n";
		return 1;
	}
	
	while (wait(NULL) > 0);
	
	LOGGER::endLogThread();
	SHAREDMEMORY::detach();
	
	bool destroy_success = IPC::destroyAll();
	if(!destroy_success) {
		std::cerr << "could not destroy IPC\n";
		return 1;
	}

	return 0;
}
