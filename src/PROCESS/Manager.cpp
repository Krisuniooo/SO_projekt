#include "../include/IPC.h"
#include "../include/Logger.h"
#include "../include/Config.h"
#include "../include/Utils.h"
#include "../include/Signals.h"

static pthread_t client_gen_thread;
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
static struct itimerval tout_val; 

bool checkConfig() {
	if(PRODUCTS < 10) {
		perror("Config error: Product types need to be equal or greater than 10");
		return false;
	}
	if((static_cast<int>(SemaphoreTypes::SEM_COUNT) - static_cast<int>(SemaphoreTypes::PRODUCTS_BASE) - 1) != PRODUCTS*3) {
		perror("Config error: N doesnt match semaphore count");
		return false;
	}
	
	if(BAKE_MIN_TIME > BAKE_MAX_TIME || BAKE_MIN_TIME < 0.0 || BAKE_MAX_TIME < 0.0) {
		perror("Config error: max baking time should be greater than min baking time and be positive");
		return false;
	}
	if(CUSTOMER_MAX_PRODUCT_DEMAND <= 0) {
		perror("Config error: Customers must demand positive number"); 
		return false;
	}
	if(CUSTOMER_MAX_PRODUCT_DEMAND <= 0) {
		perror("Config error: Customers must demand positive number"); 
		return false;
	}
	
	for(auto i : Products_base) {
		if(i.price <= 0.0 || i.max_stock <= 0 || i.max_stock > MAX_STOCK) {
			perror("Config error: bad data for products"); 
			return false;
		}
	}
	for(auto i : SemConfig) {
		if(i.value < 0) {
			perror("Config error: semaphores cant contain negative number as default"); 
			return false;
		}
	}
	
	return true;
}

bool checkOpen() {
	return (CLOSING_TIME > OPENING_TIME) ? (OPENING_TIME <= clock_time && clock_time < CLOSING_TIME) : (clock_time >= OPENING_TIME || clock_time < CLOSING_TIME);
}
bool checkRunning() {
	return (CLOSING_TIME > RUNNING_TIME) ? (RUNNING_TIME <= clock_time && clock_time < CLOSING_TIME) : (clock_time >= RUNNING_TIME || clock_time < CLOSING_TIME);
}

void add_pid(pid_t pid) {
	pthread_mutex_lock(&pid_mutex);
	active_pids.push_back(pid);
	pthread_mutex_unlock(&pid_mutex);
}

void cleanup() {
	signal(SIGTERM, SIG_IGN);
	
	keep_generating = false;
	SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::PROCESSES_MAX)); // to unlock clients if needed
	pthread_join(client_gen_thread, nullptr);

	for(pid_t p : active_pids) {
		kill(p, SIGTERM); 
	}

	for(pid_t p : active_pids) {
		int status;
		if (waitpid(p, &status, WNOHANG) == 0) {
			kill(p, SIGKILL); 
			waitpid(p, NULL, 0);
		}
	}

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
	
	if(data->is_stocktaking) {
		managerGenerateRaport();
		if(SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX))) {
			data->is_stocktaking = false;
			
			SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
		}
	}
	
	SHAREDMEMORY::detach();
	
	bool destroy_success = IPC::destroyAll();
	if(!destroy_success) {
    		perror("could not destroy IPC");
	}
}

void generateBaker() {
	if(pid_baker == -1) {
		pid_t pid = fork();
		if(pid == -1) {
			perror("IPC initialization error");
			cleanup();
			exit(1);
		} else if(pid == 0) {
			if(SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::COUT_MUTEX))) {
				printf("Generating Baker\n");
				
				SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::COUT_MUTEX));
			}
			execl("./baker", "baker", NULL);
			perror("Baker process could not be created");
			cleanup();
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
		perror("IPC initialization error");
		cleanup();
    		exit(1);
    	} else if(pid == 0) {
    		if(SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::COUT_MUTEX))) {
			printf("Generating client - Current clients inside: %d, Second register open: %d\n", data->current_customers_count, data->second_register_active);	
			SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::COUT_MUTEX));
		}
    	
    		execl("./client", "client", NULL);
    		perror("Client process could not be created!");
    		cleanup();
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
			perror("IPC initialization error");
			cleanup();
	    		exit(1);
	    	} else if(pid == 0) {
	    		if(SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::COUT_MUTEX))) {
				printf("Generating Cashier 1\n");
				
				SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::COUT_MUTEX));
			}
	    		
	    		execl("./cashier", "cashier", "1", NULL);
	    		perror("Cashier 1 process could not be created!");
	    		cleanup();
	    		exit(1);
	    	}
	    	pid_cashier1 = pid;
	} else if(pid_cashier2 == -1) {
		pid_t pid = fork();
		if(pid == -1) {
			perror("IPC initialization error");
			cleanup();
			exit(1);
		} else if(pid == 0) {
	    		if(SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::COUT_MUTEX))) {
				printf("Generating Cashier 2\n");
				
				SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::COUT_MUTEX));
			}
			execl("./cashier", "cashier", "2", NULL);
			perror("Cashier 2 process could not be created!");
			cleanup();
			exit(1);
		}
	    	pid_cashier2 = pid;
	} else {
		perror("No more cashiers could be created");
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

void managerGenerateRaport() {
	FILE *file = fopen(MANAGER_RAPORT_PATH, "w");
	
	if(!file) {
		perror("Failed to generate manager raport file");
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
	
	
	fprintf(file, "\nCASHIER - STATS:\n");
	if(SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX))) {
		for(int i=0; i<PRODUCTS; i++) {
			fprintf(file, "sold %d of %s pieces, total value of %.2f$\n", data->total_sold[i], (Products_base[i].label).c_str(), (data->total_sold[i] * Products_base[i].price));
		}
	
		SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
	}
	
	
	fclose(file);
}


void handleKillSignals(int sig) {
	if(main_pid != getpid()) {
		exit(0);
	}
	if(SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::COUT_MUTEX))) {
		printf("TRYING TO KILL\n");
		printf("CHANGED FLAGS\n");
				
		SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::COUT_MUTEX));
	}
	
	stop_program = 1;
	keep_generating = false;
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

void alarm_wakeup(int i) {
	clock_time = (clock_time + 1) % static_cast<int>(TOTAL_TIME);
	signal(SIGALRM,alarm_wakeup);
	setitimer(ITIMER_REAL, &tout_val,0);
} 

void resetTrays() {
	for(int i = 0; i<PRODUCTS; i++) {
		int max_capacity = Products_base[i].max_stock;
	
		if(SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX))) {
			if(SEMAPHORE::lock(UTILS::SEM_INDEX_MUTEX(i))) {
				int current_in_stock = data->trays[i].count;
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
	    	perror("Config error");
    		return 1;
	}

    	if (!IPC::init()) {
    		perror("IPC initialization error");
    		IPC::destroyAll();
    		return 1;
    	}
    	
	if (!LOGGER::init()) {
    		perror("LOGGER initialization error");
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
    	
    	clock_time = RUNNING_TIME; 
    	
	tout_val.it_interval.tv_sec = 1;
	tout_val.it_interval.tv_usec = 0;
	tout_val.it_value.tv_sec = 1; /* set timer for "INTERVAL seconds */
	tout_val.it_value.tv_usec = 0;
	setitimer(ITIMER_REAL, &tout_val,0);


	signal(SIGALRM,	alarm_wakeup);
    	
    	data = static_cast<SharedData*>(SHAREDMEMORY::attach());
	
   	data->is_running = true;
	
	generateBaker();
	generateCashier(); // Cashier 1
	generateCashier(); // Cashier 2
	
	if(pthread_create(&client_gen_thread, NULL, clientGeneratorRoutine, NULL) != 0) {
    		perror("Failed to create client generator thread");
		return 1;
	}
	
	while(!stop_program) {
		if(data->is_evacuation) {
			if(data->is_running || data->is_open) {
				if(SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::COUT_MUTEX))) {
					printf("EVACUATION IN PROGRESS\n");
					SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::COUT_MUTEX));
				}
				data->is_open = false;
				data->is_running = false;
				
				for(int i=0; i<PRODUCTS; i++) {
					SEMAPHORE::unlock(UTILS::SEM_INDEX_SLOTS(i));
				}
				
				terminateBaker();
				terminateCashiers();
				
				resetTrays();
			}
		}
		else {
			if(!checkOpen() && data->is_open) {
				data->is_open = false; // close for clients
				
				while(data->current_customers_count > 0) {
					SIGNALS::wait(SIGRTMIN + 2); // wait for last client
				}
				
				data->is_running = false; // close for employees
				
				for(int i=0; i<PRODUCTS; i++) {
					SEMAPHORE::unlock(UTILS::SEM_INDEX_SLOTS(i)); // wake up baker if needed
				}
				
				terminateBaker();
				terminateCashiers();
				
				resetTrays(); // reset broken semaphores from last for loop
			}
			
			if(!data->is_running && data->is_stocktaking) {
				if(SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::COUT_MUTEX))) {
					printf("GENERATING RAPORT\n");
					SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::COUT_MUTEX));
				}
				managerGenerateRaport();
				if(SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX))) {
					data->is_stocktaking = false;
				
					SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
				}
			}
			
			if(checkRunning() && !data->is_running) {
				data->is_running = true;
			
				generateBaker();
				generateCashier();
				generateCashier();
			}
			
			if(checkOpen() && !data->is_open) {
				data->is_open = true;
			}
			
			if(checkOpen() && data->is_open) {
				if(data->second_register_active && (data->current_customers_count < (MAX_CLIENT_INSIDE / 2))) {
					SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
					data->second_register_active = false;
					SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
				} else if(!data->second_register_active && (data->current_customers_count >= (MAX_CLIENT_INSIDE / 2))) {
					SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
					data->second_register_active = true;
					SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
					if(SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::COUT_MUTEX))) {
						printf("SECOND REGISTER STARTS RUNNING\n");
						SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::COUT_MUTEX));
					}
				}
			}
		}
		pause(); // wait for any signal to continue
	}
	cleanup();

	return 0;
}
