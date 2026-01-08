#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>

#include "../include/Config.h"
#include "../include/Shared.h"

void Child_proc_test() {
	SharedMemManager shm(SHARED_MEM_KEY_PATH, SHARED_MEM_KEY, sizeof(SharedData), true);

	if(shm.getAddress() == nullptr) {
		std::cerr << "Could not create shared memory\n";
		exit(1);
	}

	SharedData* data = static_cast<SharedData*>(shm.getAddress());

	data->is_running = true;
	data->is_open = true;
	data->current_customers_count = 3;
	data->today_customers_count = 30;

	sleep(2);

	std::cout << "Child proc: Sim state: " << data->is_running << "\n";
	std::cout << "Child proc: Customers count: " << data->current_customers_count << "\n";

	exit(EXIT_SUCCESS);
}

void Parent_proc_test() {
	// Just for tests
	sleep(1);

	SharedMemManager shm(SHARED_MEM_KEY_PATH, SHARED_MEM_KEY, sizeof(SharedData), false);

	if(shm.getAddress() == nullptr)  {
		std::cerr << "Could not open shared memory\n";
		exit(1);
	}


	SharedData* data = static_cast<SharedData*>(shm.getAddress());

	data->current_customers_count = 5;


	std::cout << "Parent proc: Sim state: " << data->is_running << "\n";
	std::cout << "Parent proc: Customers count: " << data->current_customers_count << "\n";

	wait(nullptr);
}

int main() {
	std::cout << sizeof(SharedData) << "\n";
	std::cout << "Number of different products: " << NUM_PRODUCTS() << "\n";

	std::cout << "Shared Memory test" << "\n";

	switch(fork()) {
		case -1:
			perror("fork error");
			exit(1);
			break;

		case 0:
			//Child
			Child_proc_test();

			break;
		default:
			//Parent

			Parent_proc_test();
	}

	return 0;
}
