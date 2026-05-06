#include "../include/pipe_server.h"
#include "../include/utils.h"
#include <windows.h>
#include <iostream>

const char* PIPE_NAME = "\\\\.\\pipe\\lab5_pipe";

PipeServer::PipeServer(FileManager& fileManager, LockManager& lockManager) : fileManager(fileManager), lockManager(lockManager){}

// Client handling
DWORD WINAPI HandleClient(LPVOID lpParam){
    // Recieving data
    ThreadData* data = static_cast<ThreadData*>(lpParam);

    HANDLE hPipe = data->hPipe;
    FileManager* fileManager = data->fileManager;
    LockManager* lockManager = data->lockManager;

    Request req;
    Response res;
    DWORD bytes;

    bool active = true;

    while(active){
        if (!ReadFile(hPipe, &req, sizeof(req), &bytes, nullptr))
        {
            break;
        }

        res.success = true;

        // Command handling
        switch (req.type) {
            // Lock while reading
            case CommandType::READ_LOCK:
            {
                if(!lockManager->lockRead(req.recordId)){
                    res.success = false;
                }
                else{
                    res.data = fileManager->readRecord(req.recordId);
                }

                // Response on read lock
                WriteFile(hPipe, &res, sizeof(res), &bytes, nullptr);

                break;
            }
            case CommandType::READ_RELEASE:
            {
                lockManager->unlockRead(req.recordId);

                break;
            }
            // Lock while writing
            case CommandType::WRITE_LOCK:
            {
                if (!lockManager->lockWrite(req.recordId)){
                    res.success = false;
                }
                else{
                    res.data = fileManager->readRecord(req.recordId);
                }

                // Response to lock
                WriteFile(hPipe, &res, sizeof(res), &bytes, nullptr);

                break;
            }
            case CommandType::WRITE_COMMIT:
            {
                fileManager->writeRecord(req.recordId, req.data);
             
                break;
            }
            case CommandType::WRITE_RELEASE:
            {
                lockManager->unlockWrite(req.recordId);

                break;
            }
            case CommandType::EXIT:
            {
                active = false;
                break;
            }
            default:
            {
                res.success = false;

                WriteFile(hPipe, &res, sizeof(res), &bytes, nullptr);

                break;
            }
        }
    }

    // Flushing pipes
    FlushFileBuffers(hPipe);
    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);
    
    delete data;

    return 0;
}

// Running pipe server
void PipeServer::run(int clientCount){
    for(int i = 0; i < clientCount; ++i){
        // Creating pipes
        HANDLE hPipe = CreateNamedPipe(
                        PIPE_NAME,
                        PIPE_ACCESS_DUPLEX,
                        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                        clientCount,
                        sizeof(Response),
                        sizeof(Request),
                        INFINITE,
                        nullptr
                        );
        
        if(hPipe == INVALID_HANDLE_VALUE){
            ThrowLastError("CreateNamedPipe failed");
        }
        
        if(!ConnectNamedPipe(hPipe, nullptr)){
            CloseHandle(hPipe);
            continue;
        }

        ThreadData* data = new ThreadData{hPipe, &fileManager, &lockManager};

        // Creating threads for clients
        HANDLE thread = CreateThread(nullptr, 0, HandleClient, data, 0, nullptr);

        if(thread == nullptr){
            delete data;
            CloseHandle(hPipe);
            continue;
        }

        CloseHandle(thread);
    }
}