#include "../include/pipe_server.h"
#include "../include/utils.h"
#include <iostream>

const char* PipeServer::PIPE_NAME = "\\\\.\\pipe\\lab5_pipe";

// Constructor
PipeServer::PipeServer(FileManager& fm, LockManager& lm)
    : fileManager(fm), lockManager(lm)
{
}

// Client handling
void PipeServer::processClient(HANDLE pipe)
{
    Request req;
    Response res;
    DWORD bytes;

    int lockedRecord = -1;
    bool writeMode = false;
    bool readMode = false;

    while (true){
        // Read request
        if (!ReadFile(pipe, &req, sizeof(req), &bytes, nullptr)) {
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
            WriteFile(pipe, &res, sizeof(res), &bytes, nullptr);
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
        
        if (!WriteFile(pipe, &res, sizeof(res), &bytes, nullptr)) {
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
    
    CloseHandle(pipe);
}

// Clients handlers thread runner
DWORD WINAPI clientThread(LPVOID param) {
    ThreadData* data = (ThreadData*)param;
    
    try {
        data->server->processClient(data->pipe);
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
    for (int i = 0; i < clientsCount; ++i)
    {
        // Create pipe
        HANDLE pipe = CreateNamedPipe(
            PIPE_NAME,
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            sizeof(Response),
            sizeof(Request),
            0,
            nullptr
        );

        if (pipe == INVALID_HANDLE_VALUE)
        {
            std::cout << "Pipe creation failed\n";
            continue;
        }

        // Try to connect
        BOOL connected = ConnectNamedPipe(pipe, nullptr);
        if (!connected)
        {
            CloseHandle(pipe);
            continue;
        }

        ThreadData* data = new ThreadData{ this, pipe };

        // Create handler thread for client
        HANDLE thread = CreateThread(
            nullptr,
            0,
            clientThread,
            data,
            0,
            nullptr
        );

        CloseHandle(thread);
    }
}