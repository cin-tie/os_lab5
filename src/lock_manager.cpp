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
        locks[i].writeSemaphore = CreateSemaphore(nullptr, 1, 1, nullptr);
        locks[i].readerMutex = CreateMutex(nullptr, FALSE, nullptr);
        locks[i].readers = 0;
        if (locks[i].writeSemaphore == nullptr || locks[i].readerMutex == nullptr)
            ThrowLastError("Failed to create semaphore or mutex");
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
    bool firstReader = (lock.readers == 1);

    // Release read lock
    ReleaseMutex(lock.readerMutex);

    // First reader acquires write semaphore (blocks writers)
    if (firstReader) {
        result = WaitForSingleObject(lock.writeSemaphore, 5000);
        if (result != WAIT_OBJECT_0) {
            WaitForSingleObject(lock.readerMutex, INFINITE);
            lock.readers--;
            ReleaseMutex(lock.readerMutex);
            
            PrintError("Failed to acquire write semaphore for read lock on record " + std::to_string(index));
            return false;
        }
    } else {
    }
    
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
    bool lastReader = (lock.readers == 0);
    
    ReleaseMutex(lock.readerMutex);

    // Last reader releases write semaphore
    if (lastReader) {
        ReleaseSemaphore(lock.writeSemaphore, 1, nullptr);
    } else {
    }
    
    return true;
}

bool LockManager::lockWrite(int index)
{
    if (index < 0 || index >= (int)locks.size()) {
        PrintError("Invalid record index for write lock: " + std::to_string(index));
        return false;
    }

    // Check write and lock
    DWORD result = WaitForSingleObject(locks[index].writeSemaphore, 5000);
    if (result != WAIT_OBJECT_0) {
        if (result == WAIT_TIMEOUT) {
            PrintError("Write lock timeout for record " + std::to_string(index) + " - record is being read by another client");
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

    // Release write semaphore
    BOOL success = ReleaseSemaphore(locks[index].writeSemaphore, 1, nullptr);
    if (!success) {
        PrintError("Failed to release write lock for record " + std::to_string(index));
    }
    return success != FALSE;
}

// Destructor
LockManager::~LockManager() {
    for (auto& lock : locks) {
        if (lock.writeSemaphore != nullptr) {
            CloseHandle(lock.writeSemaphore);
        }
        if (lock.readerMutex != nullptr) {
            CloseHandle(lock.readerMutex);
        }
    }
}