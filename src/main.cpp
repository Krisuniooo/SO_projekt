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
#include "../include/Signals.h"

static pthread_t client_gen_thread;
static pthread_t cashier_gen_thread;
static pthread_t clock_thread;
std::vector<pid_t> active_pids;
pthread_mutex_t pid_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool keep_generating = true;
static pid_t pid_baker = -1;
static pid_t pid_cashier1 = -1;
static pid_t pid_cashier2 = -1;
static SharedData* data;
static int clock_time;
static volatile sig_atomic_t stop_program = 0;
static int main_pid = -1;

bool checkConfig() {
	if((static_cast<int>(SemaphoreTypes::SEM_COUNT) - static_cast<int>(SemaphoreTypes::PRODUCTS_BASE) - 1) != PRODUCTS*3) {
		std::cerr << "Config error: N doesnt match semaphore count";
		return false;
	}
	
	return true;
}

bool checkOpen() {
	return (CLOSING_TIME > OPENING_TIME) ? (OPENING_TIME <= clock_time && clock_time < CLOSING_TIME) : (clock_time >= OPENING_TIME || clock_time < CLOSING_TIME);
}

void add_pid(pid_t pid) {
	pthread_mutex_lock(&pid_mutex);
	active_pids.push_back(pid);
	pthread_mutex_unlock(&pid_mutex);
}

void generateBaker() {
	if(pid_baker == -1) {
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
}
void terminateBaker() {
	if(pid_baker != -1) {
		kill(pid_baker, SIGTERM);
		waitpid(pid_baker, NULL, 0);
		pid_baker = -1;
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
		if(SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::PROCESSES_MAX)))
			generateClient();
		usleep(UTILS::getRandom(CUSTOMER_SPAWN_MIN_TIME, CUSTOMER_SPAWN_MAX_TIME) * SIMULATION_MINUTE); 
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

void terminateCashiers() {
	if(pid_cashier2 != -1) {
		kill(pid_cashier2, SIGTERM);
		waitpid(pid_cashier2, NULL, 0);
		pid_cashier2 = -1;
	}
	if(pid_cashier1 != -1) {
		kill(pid_cashier1, SIGTERM);
		waitpid(pid_cashier1, NULL, 0);
		pid_cashier1 = -1;
	}
}

void* cashierGeneratorRoutine(void* arg) {
	while (keep_generating) {
		if(data->is_open) {
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
	}
	return nullptr;
}

void managerGenerateRaport() {
	FILE *file = fopen(MANAGER_RAPORT_PATH, "w");
	
	if(!file) {
		std::cerr << "Failed to generate manager raport file\n";
		return;
	}
	
	char ts[32];
	UTILS::getTimestamp(ts, sizeof(ts));
	
	fprintf(file, "=== MANAGER RAPORT CREATED AT [%s] ===\n\n", ts);
	
	fprintf(file, "BAKER - STATS:\n");
	if(SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX))) {
		for(int i=0; i<PRODUCTS; i++) {
			fprintf(file, "produced %d of %s pieces\n", data->total_produced[i], (Products_base[i].label).c_str());
		}
	
		SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
	}
	fprintf(file, "\nMANAGER - LEFT ON TRAYS:\n");
	for(int i=0; i<PRODUCTS; i++) {
		fprintf(file, "left %d %s pieces on tray\n", SEMAPHORE::getValue(UTILS::SEM_INDEX_COUNT(i)), (Products_base[i].label).c_str());
	}
	
	
	//kasa
	fprintf(file, "\nCASHIER - STATS:\n");
	if(SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX))) {
		for(int i=0; i<PRODUCTS; i++) {
			fprintf(file, "sold %d of %s pieces, total value of %.2f$\n", data->total_sold[i], (Products_base[i].label).c_str(), (data->total_sold[i] * Products_base[i].price));
		}
	
		SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
	}
	
	
	fclose(file);
}

void* clockSimulationThread(void* arg) {
	clock_time = OPENING_TIME; 
	while(keep_generating) {
		usleep(SIMULATION_MINUTE);
		clock_time = (clock_time + 1) % static_cast<int>(TOTAL_TIME);
	}
	return nullptr;
}


void handleKillSignals(int sig) {
	if(main_pid != getpid()) {
		exit(0);
	}
	
	std::cout << "TRYING TO KILL\n";

	stop_program = 1;
	keep_generating = false;
	std::cout << "CHANGED FLAGS\n";
}

void handleStocktaking(int sig) {
	if(SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX))) {
		data->is_stocktaking = true;
		
		SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
	}
}

void handleEvacuation(int sig) {
	if(SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX))) {
		data->is_evacuation = !data->is_evacuation;
		
		SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
	}
}

void resetTrays() {
	for(int i = 0; i<PRODUCTS; i++) {
		int max_capacity = Products_base[i].max_stock;
	
		if(SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX))) {
			if(SEMAPHORE::lock(UTILS::SEM_INDEX_MUTEX(i))) {
				int current_in_stock = data->trays[i].count;
				std::cout << current_in_stock << " " << (max_capacity - current_in_stock) << "\n";
				SEMAPHORE::setValue(UTILS::SEM_INDEX_SLOTS(i), max_capacity - current_in_stock);
				SEMAPHORE::setValue(UTILS::SEM_INDEX_COUNT(i), current_in_stock);
				SEMAPHORE::unlock(UTILS::SEM_INDEX_MUTEX(i));
				SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
			}
		}
	}
}

int main() {
	main_pid = getpid();

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
    	
    	SIGNALS::init(SIGRTMIN + 2);
    	signal(SIGCHLD, SIG_IGN);
    	signal(SIGINT, handleKillSignals);
    	signal(SIGTERM, handleKillSignals);
    	
    	signal(SIGUSR1, handleStocktaking);
    	signal(SIGUSR2, handleEvacuation);
    	
    	data = static_cast<SharedData*>(SHAREDMEMORY::attach());
    	data->is_running = true;
	
	generateBaker();
	generateCashier(); // Cashier 1
	generateCashier(); // Cashier 2
	
	if(pthread_create(&clock_thread, NULL, clockSimulationThread, NULL) != 0) {
		std::cerr << "Failed to create clock thread\n";
		return 1;
	}
	
	/*if(pthread_create(&cashier_gen_thread, NULL, cashierGeneratorRoutine, NULL) != 0) {
		std::cerr << "Failed to create cashier generator thread\n";
		return 1;
	}*/
	
	data->is_open = true;
	
	if(pthread_create(&client_gen_thread, NULL, clientGeneratorRoutine, NULL) != 0) {
		std::cerr << "Failed to create client generator thread\n";
		return 1;
	}
	
	while(!stop_program) {
		if(!checkOpen() && data->is_open) {
			data->is_open = false;
			std::cout << "A\n";
			
			if(data->current_customers_count <=0) {
				SIGNALS::wait(SIGRTMIN + 2);
			}
			
			std::cout << "B\n";
			data->is_running = false;
			
			for(int i=0; i<PRODUCTS; i++) {
				SEMAPHORE::unlock(UTILS::SEM_INDEX_SLOTS(i)); // wake up baker if needed
			}
			
			terminateBaker();
			terminateCashiers();
			
			std::cout << "C\n";
			
			resetTrays(); // reset broken semaphores from last for loop
		}
		
		if(!data->is_open && data->is_stocktaking && (data->current_customers_count <=0)) {
			std::cout << "GENERATING RAPOR\n";
			managerGenerateRaport();
			if(SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX))) {
				data->is_stocktaking = false;
			
				SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
			}
		}
		if(checkOpen() && !data->is_open) {
			
			data->is_running = true;
			data->is_open = true;
		
			generateBaker();
			generateCashier();
			generateCashier();
		}
		sleep(1);
		
	}
	
	signal(SIGTERM, SIG_IGN);
	
	std::cout << "A";
	
	keep_generating = false;
	SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::PROCESSES_MAX));
	pthread_join(client_gen_thread, nullptr);
	std::cout << "B";

	for(pid_t p : active_pids) {
		kill(p, SIGTERM); 
	}
	std::cout << "C";

	for(pid_t p : active_pids) {
		int status;
		if (waitpid(p, &status, WNOHANG) == 0) {
			kill(p, SIGKILL); 
			waitpid(p, NULL, 0);
		}
	}
	std::cout << "D";

	if(pid_baker != -1) {
		kill(pid_baker, SIGTERM);
		waitpid(pid_baker, NULL, 0);
	}
	if (pid_cashier1 != -1) {
		kill(pid_cashier1, SIGTERM);
		waitpid(pid_cashier1, NULL, 0);
	}
	if (pid_cashier2 != -1) {
		kill(pid_cashier2, SIGTERM);
		waitpid(pid_cashier2, NULL, 0);
	}
	std::cout << "E";
	
	if(data->is_stocktaking) {
		managerGenerateRaport();
		if(SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX))) {
			data->is_stocktaking = false;
			
			SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
		}
	}
	std::cout << "F";
	pthread_join(clock_thread, nullptr);
	std::cout << "G";
	
	SHAREDMEMORY::detach();
	
	bool destroy_success = IPC::destroyAll();
	if(!destroy_success) {
		std::cerr << "could not destroy IPC\n";
	}
	std::cout << "H";


	return 0;
}
