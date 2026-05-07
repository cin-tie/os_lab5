#pragma once

#include <windows.h>
#include <vector>

// Struct for every employee to lock
struct RecordLock
{
    HANDLE writeSemaphore;
    HANDLE readerMutex;
    LONG readers;
};

class LockManager
{
private:
    std::vector<RecordLock> locks;

public:
    LockManager(int count);

    bool lockRead(int index);
    bool unlockRead(int index);

    bool lockWrite(int index);
    bool unlockWrite(int index);

    ~LockManager();
};