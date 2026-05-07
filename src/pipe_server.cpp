#include "../include/pipe_server.h"
#include "../include/utils.h"
#include <iostream>

const char* PipeServer::PIPE_NAME = "\\\\.\\pipe\\lab5_pipe";

// Constructor
PipeServer::PipeServer(FileManager& fileManager, LockManager& lockManager)
    : fileManager(fileManager), lockManager(lockManager)
{
}

// Client handling
void PipeServer::processClient(ThreadData* threadData)
{
    Request req;
    Response res;
    DWORD bytes;

    int lockedRecord = -1;
    bool writeMode = false;
    bool readMode = false;

    while (true){
        // Read request
        if (!ReadFile(threadData->hPipe, &req, sizeof(req), &bytes, nullptr)) {
            DWORD error = GetLastError();
            if (error != ERROR_BROKEN_PIPE) {
                PrintError("ReadFile failed with error: " + std::to_string(error));
            }
            break;
        }

        if (bytes != sizeof(req)) {
            PrintError("Invalid request size received");
            continue;
        }
        
        if (req.type == CommandType::EXIT) {
            break;
        }

        int index = req.recordId;
        res.success = false;

        // Validate record index
        int recordCount = fileManager.getRecordCount();
        if(index < 0 || index > recordCount){
            PrintError("Invalid record index: " + std::to_string(index) + 
                      " (available: 0-" + std::to_string(recordCount - 1) + ")");
            res.success = false;
            WriteFile(threadData->hPipe, &res, sizeof(res), &bytes, nullptr);
            continue;
        }

        // Command handling
        switch (req.type)
        {
        case CommandType::READ_LOCK:
        {
            if (!lockManager.lockRead(index)) {
                res.success = false;
                PrintError("Failed to acquire read lock for record " + std::to_string(index));
                break;
            }

            try {
                lockedRecord = index;
                readMode = true;

                res.success = true;
                res.data = fileManager.readRecord(index);
            } catch (const std::exception& e) {
                PrintError("Error reading record: " + std::string(e.what()));
                lockManager.unlockRead(index);
                lockedRecord = -1;
                readMode = false;
                res.success = false;
            }

            break;
        }

        case CommandType::READ_RELEASE:
        {
            if (readMode && lockedRecord != -1) {
                lockManager.unlockRead(lockedRecord);
                readMode = false;
                lockedRecord = -1;
            }

            continue;
        }

        case CommandType::WRITE_LOCK:{
            if (!lockManager.lockWrite(index)) {
                res.success = false;
                PrintError("Failed to acquire write lock for record " + std::to_string(index) + 
                          " - record is being read or written");
                break;
            }

            try {
                lockedRecord = index;
                writeMode = true;

                res.success = true;
                res.data = fileManager.readRecord(index);
            } catch (const std::exception& e) {
                PrintError("Error reading record for write: " + std::string(e.what()));
                lockManager.unlockWrite(index);
                lockedRecord = -1;
                writeMode = false;
                res.success = false;
            }

            break;
        }

        case CommandType::WRITE_COMMIT:
        {
            if (writeMode && lockedRecord != -1) {
                if (IsValidEmployee(req.data)) {
                    try {
                        fileManager.writeRecord(lockedRecord, req.data);

                        res.success = true;
                        res.data = fileManager.readRecord(lockedRecord);
                    } catch (const std::exception& e) {
                        PrintError("Error committing write: " + std::string(e.what()));
                        res.success = false;
                    }
                } else {
                    PrintError("Invalid employee data provided for commit");
                    res.success = false;
                }
            } else {
                PrintError("Write commit attempted without active write lock");
                res.success = false;
            }

            break;
        }

        case CommandType::WRITE_RELEASE:
        {
            if (writeMode && lockedRecord != -1) {
                lockManager.unlockWrite(lockedRecord);
                writeMode = false;
                lockedRecord = -1;
            }

            continue;
        }

        default:
            PrintError("Unknown command type received");
            continue;
        }
        
        if (!WriteFile(threadData->hPipe, &res, sizeof(res), &bytes, nullptr)) {
            PrintError("Failed to write response to pipe");
            break;
        }

    }

    // Clean up if client disconnected without releasing locks
    if (readMode && lockedRecord != -1) {
        lockManager.unlockRead(lockedRecord);
    }
    if (writeMode && lockedRecord != -1) {
        lockManager.unlockWrite(lockedRecord);
    }
    
    CloseHandle(threadData->hPipe);

    std::cout << "Client " << threadData->clientNumber << " turned off..." << std::endl;
}

// Clients handlers thread runner
DWORD WINAPI clientThread(LPVOID param) {
    ThreadData* data = (ThreadData*)param;
    
    try {
        data->server->processClient(data);
    } catch(...) // Catch any exception while running client
    {
        PrintError("Exception in client thread");
    }
    
    delete data;
    return 0;
}

// Server run
void PipeServer::run(int clientsCount)
{
    HANDLE* threads = new HANDLE[clientsCount];

    for (int i = 0; i < clientsCount; ++i)
    {
        // Create pipe
        HANDLE hPipe = CreateNamedPipe(
            PIPE_NAME,
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            sizeof(Response),
            sizeof(Request),
            5000,
            nullptr
        );

        if (hPipe == INVALID_HANDLE_VALUE) {
            PrintError("Pipe creation failed");
            continue;
        }

        std::cout << "Waiting for client " << (i + 1) << " to connect..." << std::endl;

        // Try to connect
        BOOL connected = ConnectNamedPipe(hPipe, nullptr);
        if (!connected && GetLastError() != ERROR_PIPE_CONNECTED) {
            PrintError("Failed to connect pipe");
            CloseHandle(hPipe);
            continue;
        }

        ThreadData* data = new ThreadData{ this, hPipe, i + 1 };

        // Create handler thread for client
        HANDLE thread = CreateThread(
            nullptr,
            0,
            clientThread,
            data,
            0,
            nullptr
        );

        if (threads[i] == nullptr) {
            PrintError("Failed to create client thread");
            delete data;
            CloseHandle(hPipe);
        }

    }

    // Wait for all client threads to complete
    WaitForMultipleObjects(clientsCount, threads, TRUE, INFINITE);
    
    for (int i = 0; i < clientsCount; ++i) {
        if (threads[i] != nullptr) {
            CloseHandle(threads[i]);
        }
    }
    
    delete[] threads;

    std::cout << "Pipe server turned off..." << std::endl;
}