#include "../include/pipe_server.h"
#include "../include/utils.h"
#include <windows.h>
#include <iostream>

const char* PIPE_NAME = "\\\\.\\pipe\\lab5_pipe";

PipeServer::PipeServer(FileManager& fileManager, LockManager& lockManager) : fileManager(fileManager), lockManager(lockManager){}
