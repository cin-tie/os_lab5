#include "../include/lock_manager.h"
#include <stdexcept>

// Initialization
LockManager::LockManager(int count){
    if (count <= 0) {
        return;
    }
    
    locks.resize(count);

    for(int i = 0; i < count; ++i){
        locks[i].resourceMutex = CreateMutex(nullptr, FALSE, ("WRITE_RESOURCES_MUTEX_" + std::to_string(i)).c_str());
        locks[i].readerMutex = CreateMutex(nullptr, FALSE, ("READ_RESOURCES_MUTEX_" + std::to_string(i)).c_str());
        locks[i].readers = 0;
    }
}

// Lock reading
void LockManager::lockRead(int index){
    if (index < 0 || index >= static_cast<int>(locks.size())) {
        return;
    }
    
    RecordLock& lock = locks[index];

    WaitForSingleObject(lock.readerMutex, INFINITE);

    lock.readers++;

    if(lock.readers == 1){
        WaitForSingleObject(lock.resourceMutex, INFINITE);
    }

    ReleaseMutex(lock.readerMutex);
}

// Unlock reading
void LockManager::unlockRead(int index){
    if (index < 0 || index >= static_cast<int>(locks.size())) {
        return;
    }
    
    RecordLock& lock = locks[index];
    
    WaitForSingleObject(lock.readerMutex, INFINITE);

    lock.readers--;

    if(lock.readers == 0){
        ReleaseMutex(lock.resourceMutex);
    }
    
    ReleaseMutex(lock.readerMutex);
}

// Lock writing
void LockManager::lockWrite(int index){
    if (index < 0 || index >= static_cast<int>(locks.size())) {
        return;
    }
    
    WaitForSingleObject(locks[index].resourceMutex, INFINITE);
}

// Unlock writing
void LockManager::unlockWrite(int index){
    if (index < 0 || index >= static_cast<int>(locks.size())) {
        return;
    }
    
    ReleaseMutex(locks[index].resourceMutex);
}

// Destructor
LockManager::~LockManager() {
    for(size_t i = 0; i < locks.size(); ++i){
        if (locks[i].resourceMutex) {
            CloseHandle(locks[i].resourceMutex);
        }
        if (locks[i].readerMutex) {
            CloseHandle(locks[i].readerMutex);
        }
    }
}