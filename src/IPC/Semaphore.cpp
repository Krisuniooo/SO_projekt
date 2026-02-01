#include "../../include/IPC/Semaphore.h"
#include "../../include/Config.h"
#include "../../include/Utils.h"

static int sem_id = -1;

bool SEMAPHORE::init() {
	bool setupSuccess = UTILS::setupKeyFile(SEMAPHORE_KEY_PATH);
	
	if(setupSuccess == false) {
		perror("Could not create key file");
		return false;
	}
	
	bool createSuccess = SEMAPHORE::create();
	
	if(createSuccess == false) {
		perror("could not create set of semaphores");
		return false;
	}
	
	for (int i = 0; i < static_cast<int>(SemaphoreTypes::SEM_COUNT); i++) {
		int val = 1;
		
		for(auto j : SemConfig) {
			if (static_cast<int>(j.type) == i) {
				val = j.value;
			}
		}
		SEMAPHORE::setValue(i, val);
		
	}
	
	for(int i = 0; i<PRODUCTS; i++) {
		SEMAPHORE::setValue(UTILS::SEM_INDEX_MUTEX(i), 1); //mutex
		SEMAPHORE::setValue(UTILS::SEM_INDEX_SLOTS(i), Products_base[i].max_stock);
		SEMAPHORE::setValue(UTILS::SEM_INDEX_COUNT(i), 0);
	}
	
	return true;
}

bool SEMAPHORE::create() {
	sem_id = SEMAPHORE::getID(IPC_CREAT | IPC_EXCL | 0600);
	
	if(sem_id == -1) return false;
	
	return true;
}

int SEMAPHORE::getID(int flags) {
	if(sem_id != -1) return sem_id;

	key_t key = ftok(SEMAPHORE_KEY_PATH, SEMAPHORE_KEY);

	if(key == -1) {
		perror("ftok Error");
		return -1;
	}
	
	sem_id = semget(key, static_cast<int>(SemaphoreTypes::SEM_COUNT), flags);
	
	if(sem_id == -1) {
		perror("semget Error");
	}
	
	return sem_id;
}


bool SEMAPHORE::lock(int sem_num, bool nowait) {
	if(sem_id == -1 && SEMAPHORE::getID() == -1) return false;

	struct sembuf sop;
	
	sop.sem_num = sem_num; 
	sop.sem_op = -1;
	sop.sem_flg = nowait ? IPC_NOWAIT : 0;
	
	if(semop(sem_id, &sop, 1) == -1) {
		if(errno == EAGAIN || (nowait && errno == EWOULDBLOCK)) 
			return false;
		if(errno != EINTR && errno != EIDRM) {
			perror("semop Error");
			return false;
		}
	}
	
	return true;
}

bool SEMAPHORE::unlock(int sem_num) {
	if(sem_id == -1 && SEMAPHORE::getID() == -1) return false;

	struct sembuf sop;
	
	sop.sem_num = sem_num; 
	sop.sem_op = 1;
	sop.sem_flg = 0;
	
	if(semop(sem_id, &sop, 1) == -1) {
		perror("semop Error");
		return false;
	}
	
	return true;
}

void SEMAPHORE::setValue(int sem_num, int new_val) {
	if(sem_id == -1 && SEMAPHORE::getID() == -1) return;
	
	union semun {
            int val;
            struct semid_ds *buf;
            unsigned short *array;
        } arg;
        
        arg.val = new_val;

	int val = semctl(sem_id, sem_num, SETVAL, arg);
	if(val == -1) {
		perror("semctl Error (SETVAL)");
	}
}

int SEMAPHORE::getValue(int sem_num) {
	if(sem_id == -1 && SEMAPHORE::getID() == -1) return false;
	
	int value = semctl(sem_id, sem_num, GETVAL);
	if(value == -1) {
		perror("semctl Error (GETVAL)");
	}
	
	return value;
}

bool SEMAPHORE::destroy() {
	if(sem_id == -1) {
		perror("semctl Error (IPC_RMID): Semaphore not initialized");
		return false;
	} 
	
	if(semctl(sem_id, 0, IPC_RMID, 0) == -1) {
		perror("semctl Error (IPC_RMID): could not remove set");
		return false;
	}
	
	sem_id = -1;
	return true;
}
