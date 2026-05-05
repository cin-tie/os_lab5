#pragma once

#include <windows.h>
#include <string>
#include <vector>

// Locker structure
struct RecordLock
{
    HANDLE resourceMutex;
    HANDLE readerMutex;
    int readers;
};

// Lock manager
class LockManager
{
private:
    std::vector<RecordLock> locks;

public:
    LockManager(int count);
    
    void lockRead(int index);
    void unlockRead(int index);
    void lockWrite(int index);
    void unlockWrite(int index);

    ~LockManager();
};