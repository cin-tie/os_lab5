#include "../include/lock_manager.h"

// Constructor
LockManager::LockManager(int count)
{
    locks.resize(count);

    for (int i = 0; i < count; ++i)
    {
        locks[i].resourceMutex = CreateMutex(nullptr, FALSE, nullptr);
        locks[i].readerMutex = CreateMutex(nullptr, FALSE, nullptr);
        locks[i].readers = 0;
    }
}

// Locking read
bool LockManager::lockRead(int index)
{
    auto& lock = locks[index];

    // Check read is ready and lock
    if(WaitForSingleObject(lock.readerMutex, 5000) !=  WAIT_OBJECT_0){
        return false;
    }

    lock.readers++;

    // Check nobody writing and lock
    if (lock.readers == 1){
        if(WaitForSingleObject(lock.resourceMutex, 5000) != WAIT_OBJECT_0){
            lock.readers--;
            ReleaseMutex(lock.readerMutex);
            return false;
        }
    }

    // Release read lock
    ReleaseMutex(lock.readerMutex);
    
    return true;
}

bool LockManager::unlockRead(int index)
{
    auto& lock = locks[index];

    // Check read is ready and lock
    if(WaitForSingleObject(lock.readerMutex, 5000) != WAIT_OBJECT_0){
        return false;
    }

    lock.readers--;

    // Release writing if nobody reading
    if (lock.readers == 0)
        ReleaseMutex(lock.resourceMutex);

    // Release read lock
    return ReleaseMutex(lock.readerMutex);

}

bool LockManager::lockWrite(int index)
{
    // Check write and lock
    if(WaitForSingleObject(locks[index].resourceMutex, 5000) != WAIT_OBJECT_0)
        return false;

    return true;
}

bool LockManager::unlockWrite(int index)
{
    // Release write lock
    return ReleaseMutex(locks[index].resourceMutex);
}

// Destructor
LockManager::~LockManager()
{
    for (auto& lock : locks)
    {
        CloseHandle(lock.resourceMutex);
        CloseHandle(lock.readerMutex);
    }
}