#pragma once

#include <windows.h>
#include <string>
#include "file_manager.h"
#include "lock_manager.h"
#include "protocol.h"

class PipeServer
{
private:
    FileManager& fileManager;
    LockManager& lockManager;

    static const char* PIPE_NAME;

    
public:
    PipeServer(FileManager& fm, LockManager& lm);
    void processClient(HANDLE pipe);

    void run(int clientsCount);
};

struct ThreadData {
    PipeServer* server;
    HANDLE pipe;
};