#include "../include/lock_manager.h"
#include "../include/utils.h"

// Constructor
LockManager::LockManager(int count)
{
    if (count <= 0) {
        throw std::invalid_argument("LockManager: count must be positive");
    }
    
    locks.resize(count);

    for (int i = 0; i < count; ++i)
    {
        locks[i].resourceMutex = CreateMutex(nullptr, FALSE, nullptr);
        locks[i].readerMutex = CreateMutex(nullptr, FALSE, nullptr);
        
        if (locks[i].resourceMutex == nullptr || locks[i].readerMutex == nullptr) {
            ThrowLastError("Failed to create mutex");
        }
        
        locks[i].readers = 0;
    }
}

// Locking read
bool LockManager::lockRead(int index)
{
    if (index < 0 || index >= (int)locks.size()) {
        PrintError("Invalid record index for read lock: " + std::to_string(index));
        return false;
    }

    auto& lock = locks[index];

    // Check read is ready and lock
    DWORD result = WaitForSingleObject(lock.readerMutex, 5000);
    if (result != WAIT_OBJECT_0) {
        if (result == WAIT_TIMEOUT) {
            PrintError("Read lock timeout for record " + std::to_string(index));
        } else {
            PrintError("Failed to acquire reader mutex for record " + std::to_string(index));
        }
        return false;
    }

    lock.readers++;

    // Check nobody writing and lock
    if (lock.readers == 1) {
        result = WaitForSingleObject(lock.resourceMutex, 5000);
        if (result != WAIT_OBJECT_0) {
            lock.readers--;
            ReleaseMutex(lock.readerMutex);
            PrintError("Failed to acquire resource mutex for read lock on record " + std::to_string(index));
            return false;
        }
    }

    // Release read lock
    ReleaseMutex(lock.readerMutex);
    
    return true;
}

bool LockManager::unlockRead(int index)
{
    if (index < 0 || index >= (int)locks.size()) {
        PrintError("Invalid record index for read unlock: " + std::to_string(index));
        return false;
    }

    auto& lock = locks[index];

    // Check read is ready and lock
    DWORD result = WaitForSingleObject(lock.readerMutex, 5000);
    if (result != WAIT_OBJECT_0) {
        PrintError("Failed to acquire reader mutex for read unlock on record " + std::to_string(index));
        return false;
    }

    lock.readers--;

    // Release writing if nobody reading
    if (lock.readers == 0) {
        ReleaseMutex(lock.resourceMutex);
    }

    // Release read lock
    BOOL success = ReleaseMutex(lock.readerMutex);
    return success != FALSE;

}

bool LockManager::lockWrite(int index)
{
    if (index < 0 || index >= (int)locks.size()) {
        PrintError("Invalid record index for write lock: " + std::to_string(index));
        return false;
    }

    // Check write and lock
    DWORD result = WaitForSingleObject(locks[index].resourceMutex, 5000);
    if (result != WAIT_OBJECT_0) {
        if (result == WAIT_TIMEOUT) {
            PrintError("Write lock timeout for record " + std::to_string(index) + " - record is locked by another client");
        }
        return false;
    }
    
    return true;
}

bool LockManager::unlockWrite(int index)
{
    if (index < 0 || index >= (int)locks.size()) {
        PrintError("Invalid record index for write unlock: " + std::to_string(index));
        return false;
    }

    // Release write lock
    BOOL success = ReleaseMutex(locks[index].resourceMutex);
    if (!success) {
        PrintError("Failed to release write lock for record " + std::to_string(index));
    }
    return success != FALSE;
}

// Destructor
LockManager::~LockManager() {
    for (auto& lock : locks) {
        if (lock.resourceMutex != nullptr) {
            CloseHandle(lock.resourceMutex);
        }
        if (lock.readerMutex != nullptr) {
            CloseHandle(lock.readerMutex);
        }
    }
}