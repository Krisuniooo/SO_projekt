#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <signal.h>
#include <vector>
#include <fcntl.h>
#include <sys/stat.h>

#include "../include/IPC.h"
#include "../include/Logger.h"
#include "../include/Config.h"
#include "../include/Utils.h"

static pthread_t client_gen_thread;
static pthread_t cashier_gen_thread;
std::vector<pid_t> active_pids;
pthread_mutex_t pid_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool keep_generating = true;
static pid_t pid_baker = -1;
static pid_t pid_cashier1 = -1;
static pid_t pid_cashier2 = -1;
static SharedData* data;

bool checkConfig() {
	if((static_cast<int>(SemaphoreTypes::SEM_COUNT) - static_cast<int>(SemaphoreTypes::PRODUCTS_BASE) - 1) != PRODUCTS*3) {
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
    	pid_baker = pid;
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
		sleep(UTILS::getRandom(CUSTOMER_SPAWN_MIN_TIME, CUSTOMER_SPAWN_MAX_TIME)); 
		cleanup_zombies();
	}
	return nullptr;
}

void generateCashier() {
	if(pid_cashier1 == -1) {
		pid_t pid = fork();
		if(pid == -1) {
	    		std::cerr << "IPC initialization error \n";
	    		exit(1);
	    	} else if(pid == 0) {
	    		std::cout << "Generating Cashier 1\n";
	    		execl("./cashier", "cashier", "1", NULL);
	    		std::cerr << "Cashier 1 process could not be created! \n";
	    		exit(1);
	    	}
	    	pid_cashier1 = pid;
	} else if(pid_cashier2 == -1) {
		pid_t pid = fork();
		if(pid == -1) {
			std::cerr << "IPC initialization error \n";
			exit(1);
		} else if(pid == 0) {
			std::cout << "Generating Cashier 2\n";
			execl("./cashier", "cashier", "2", NULL);
			std::cerr << "Cashier 2 process could not be created! \n";
			exit(1);
		}
	    	pid_cashier2 = pid;
	} else {
		std::cerr << "No more cashiers could be created\n";
	}
}

void* cashierGeneratorRoutine(void* arg) {
	SharedData* data = static_cast<SharedData*>(SHAREDMEMORY::attach());
	while (true) {
		if(pid_cashier2 == -1) {
			SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
			int customers_inside = data->current_customers_count;
			SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
			
			if(customers_inside >= (MAX_CLIENT_INSIDE / 2)) {
				generateCashier();
			}
			
			sleep(1);
		} else {
			waitpid(pid_cashier2, NULL, 0);
			pid_cashier2 = -1;
		}
	}
	SHAREDMEMORY::detach();
	return nullptr;
}

void handleKillSignals(int sig) {
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
	if (pid_cashier1 != -1) 
		kill(pid_cashier1, SIGTERM);
	if (pid_cashier2 != -1) 
		kill(pid_cashier2, SIGTERM);
	
	SHAREDMEMORY::detach();
	
	bool destroy_success = IPC::destroyAll();
	if(!destroy_success) {
		std::cerr << "could not destroy IPC\n";
	}
	
	exit(1);
}

void handleStocktaking(int sig) { }
void handleEvacuation(int sig) { }

int main() {
	if (!checkConfig()) {
	    	std::cerr << "Config error \n";
    		return 1;
	}

    	if (!IPC::init()) {
    		std::cerr << "IPC initialization error \n";
    		IPC::destroyAll();
    		return 1;
    	}
    	
	if (!LOGGER::init()) {
    		std::cerr << "LOGGER initialization error \n";
    		IPC::destroyAll();
    		return 1;
    	}
    	LOGGER::log("Initialization successful\n");
    	
    	signal(SIGCHLD, SIG_IGN);
    	signal(SIGINT, handleKillSignals);
    	signal(SIGTERM, handleKillSignals);
    	
    	signal(SIGUSR1, handleStocktaking);
    	signal(SIGUSR2, handleEvacuation);
    	
    	data = static_cast<SharedData*>(SHAREDMEMORY::attach());
    	data->is_running = true;
	
	generateBaker();
	generateCashier(); // Cashier 1
	
	if(pthread_create(&cashier_gen_thread, NULL, cashierGeneratorRoutine, NULL) != 0) {
		std::cerr << "Failed to create cashier generator thread\n";
		return 1;
	}
	
	if(pthread_create(&client_gen_thread, NULL, clientGeneratorRoutine, NULL) != 0) {
		std::cerr << "Failed to create client generator thread\n";
		return 1;
	}
	
	while (wait(NULL) > 0);
	
	SHAREDMEMORY::detach();
	
	bool destroy_success = IPC::destroyAll();
	if(!destroy_success) {
		std::cerr << "could not destroy IPC\n";
		return 1;
	}

	return 0;
}
