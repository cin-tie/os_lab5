#include "../include/lock_manager.h"

// Initialization
LockManager::LockManager(int count){
    locks.resize(count);

    for(int i = 0; i < count; ++i){
        locks[i].resourceMutex = CreateMutex(nullptr, FALSE, ("WRITE_RESOURCES_MUTEX_" + std::to_string(i)).c_str());
        locks[i].readerMutex = CreateMutex(nullptr, FALSE, ("READ_RESOURCES_MUTEX_" + std::to_string(i)).c_str());
        locks[i].readers = 0;
    }
}

// Lock reading
// Waiting for reading available, locking writing if not already
void LockManager::lockRead(int index){
    RecordLock& lock = locks[index];

    WaitForSingleObject(lock.readerMutex, INFINITE);

    lock.readers++;

    if(lock.readers == 1){      // First reading blocks writing
        WaitForSingleObject(lock.resourceMutex, INFINITE);
    }

    ReleaseMutex(lock.readerMutex);
}

// Unlock reading
void LockManager::unlockRead(int index){
    RecordLock& lock = locks[index];
    
    WaitForSingleObject(lock.readerMutex, INFINITE);

    lock.readers--;

    if(lock.readers == 0){
        ReleaseMutex(lock.resourceMutex);
    }
    
    ReleaseMutex(lock.readerMutex);
}

// Lock writing
// Waiting for writing available
void LockManager::lockWrite(int index){
    WaitForSingleObject(locks[index].resourceMutex, INFINITE);
}

// Unlock writing
void LockManager::unlockWrite(int index){
    ReleaseMutex(locks[index].resourceMutex);
}

// Destructor
LockManager::~LockManager() {
    for(int i = 0; i < locks.size(); ++i){
        CloseHandle(locks[i].resourceMutex);
        CloseHandle(locks[i].readerMutex);
    }
}