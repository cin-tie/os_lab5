#pragma once

#include "file_manager.h"
#include "lock_manager.h"
#include "protocol.h"

// Pipe server for responding client
class PipeServer
{
private:
    FileManager& fileManager;
    LockManager& lockManager;

public:
    PipeServer(FileManager& fileManager, LockManager& lockManager);

    void run(int clientCount);
};

// Thread data
struct ThreadData
{
    HANDLE hPipe;
    FileManager* fileManager;
    LockManager* lockManager;
};
