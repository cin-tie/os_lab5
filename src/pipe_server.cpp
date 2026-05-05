#include "../include/pipe_server.h"
#include "../include/utils.h"
#include <windows.h>
#include <iostream>

const char* PIPE_NAME = "\\\\.\\pipe\\lab5_pipe";

PipeServer::PipeServer(FileManager& fileManager, LockManager& lockManager) : fileManager(fileManager), lockManager(lockManager){}

// Thread function for handling client requests
DWORD WINAPI HandleClient(LPVOID lpParam){
    ClientContext* context = static_cast<ClientContext*>(lpParam);

    HANDLE hPipe = context->pipe;
    FileManager* fileManager = context->fileManager;
    LockManager* lockManager = context->lockManager;

    BOOL connected = ConnectNamedPipe(hPipe, nullptr) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

    if (!connected) {
        std::cerr << "Failed to connect client pipe" << std::endl;
        CloseHandle(hPipe);
        delete context;
        return 1;
    }

    Request request;
    Response response;
    
    DWORD bytesRead = 0;
    DWORD bytesWritten = 0;

    while(true){
        memset(&request, 0, sizeof(request));
        
        BOOL readResult = ReadFile(hPipe, &request, sizeof(request), &bytesRead, nullptr);

        if(!readResult || bytesRead == 0){
            if (GetLastError() != ERROR_BROKEN_PIPE) {
                std::cerr << "ReadFile error: " << GetLastError() << std::endl;
            }
            break;
        }

        if (request.recordId < 0 || request.recordId >= fileManager->getRecordCount()){
            response.success = false;
            memset(&response.data, 0, sizeof(employee));

            if (!WriteFile(
                hPipe,
                &response,
                sizeof(response),
                &bytesWritten,
                nullptr
            )) {
                std::cerr << "Failed to send error response" << std::endl;
            }
            continue;
        }

        switch (request.type)
        {
        case CommandType::READ:
            lockManager->lockRead(request.recordId);

            response.data = fileManager->readRecord(request.recordId);
            response.success = true;

            if (!WriteFile(hPipe, &response, sizeof(response), &bytesWritten, nullptr)) {
                std::cerr << "Failed to send read response" << std::endl;
            }

            lockManager->unlockRead(request.recordId);

            break;
        case CommandType::WRITE:
            lockManager->lockWrite(request.recordId);

            fileManager->writeRecord(request.recordId, request.data);
            response.success = true;

            if (!WriteFile(hPipe, &response, sizeof(response), &bytesWritten, nullptr)) {
                std::cerr << "Failed to send write response" << std::endl;
            }

            lockManager->unlockWrite(request.recordId);

            break;
        case CommandType::EXIT:
            break;
        
        default:
            response.success = false;
            WriteFile(hPipe, &response, sizeof(response), &bytesWritten, nullptr);
            break;
        }

        if(request.type == CommandType::EXIT){
            break;
        }
    }

    // Cleanup
    FlushFileBuffers(hPipe);
    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);

    delete context;

    return 0;
}

// Main server loop - creates named pipes and handles client connections
void PipeServer::run(int clientCount){
    std::vector<HANDLE> threadHandles;

    for(int i = 0; i < clientCount; ++i){
        HANDLE hPipe = CreateNamedPipe(
            PIPE_NAME,
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            sizeof(Response),
            sizeof(Request),
            0,
            nullptr
        );

        if(hPipe == INVALID_HANDLE_VALUE){
            ThrowLastError("CreateNamedPipe failed");
        }

        ClientContext* context = new ClientContext{hPipe, &fileManager, &lockManager};

        HANDLE threadHandle = CreateThread(nullptr, 0, HandleClient, context, 0, nullptr);

        if(threadHandle == nullptr){
            delete context;
            CloseHandle(hPipe);

            ThrowLastError("CreateThread failed");
        }

        threadHandles.push_back(threadHandle);
    }
    
    WaitForMultipleObjects(static_cast<DWORD>(threadHandles.size()), threadHandles.data(), TRUE, INFINITE);

    for(HANDLE hThread : threadHandles){
        CloseHandle(hThread);
    }
}