#pragma once

#include "file_manager.h"
#include "lock_manager.h"
#include "protocol.h"

// Struct for thread creation
struct ClientContext {
    HANDLE pipe;
    FileManager* fileManager;
    LockManager* lockManager;
};

// Pipe server class
class PipeServer {
private:
    FileManager& fileManager;
    LockManager& lockManager;

public:
    PipeServer(FileManager& fileManager, LockManager& lockManager);
    
    void run(int clientCount);
};