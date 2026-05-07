#include "../include/pipe_server.h"
#include <iostream>

const char* PipeServer::PIPE_NAME = "\\\\.\\pipe\\lab5_pipe";

PipeServer::PipeServer(FileManager& fm, LockManager& lm)
    : fileManager(fm), lockManager(lm)
{
}

void PipeServer::processClient(HANDLE pipe)
{
    Request req;
    Response res;
    DWORD bytes;

    int lockedRecord = -1;
    bool writeMode = false;
    bool readMode = false;

    while (true)
    {
        if (!ReadFile(pipe, &req, sizeof(req), &bytes, nullptr))
            break;

        if (req.type == CommandType::EXIT)
            break;

        int index = req.recordId;

        if (index == -1)
        {
            res.success = false;
            WriteFile(pipe, &res, sizeof(res), &bytes, nullptr);
            continue;
        }

        switch (req.type)
        {
        case CommandType::READ_LOCK:
        {
            if (!lockManager.lockRead(index))
            {
                res.success = false;
                break;
            }

            lockedRecord = index;
            readMode = true;

            res.success = true;
            res.data = fileManager.readRecord(index);
            break;
        }

        case CommandType::READ_RELEASE:
        {
            if (readMode)
            {
                lockManager.unlockRead(lockedRecord);
                readMode = false;
                lockedRecord = -1;
            }

            continue;
        }

        case CommandType::WRITE_LOCK:
        {
            if (!lockManager.lockWrite(index))
            {
                res.success = false;
                break;
            }

            lockedRecord = index;
            writeMode = true;

            res.success = true;
            res.data = fileManager.readRecord(index);
            break;
        }

        case CommandType::WRITE_COMMIT:
        {
            if (writeMode && lockedRecord != -1)
            {
                fileManager.writeRecord(lockedRecord, req.data);
                res.success = true;
                res.data = fileManager.readRecord(lockedRecord);
            }
            else
            {
                res.success = false;
            }

            break;
        }

        case CommandType::WRITE_RELEASE:
        {
            if (writeMode)
            {
                lockManager.unlockWrite(lockedRecord);
                writeMode = false;
                lockedRecord = -1;
            }

            continue;
        }

        default:
            continue;
        }

        WriteFile(pipe, &res, sizeof(res), &bytes, nullptr);
    }

    CloseHandle(pipe);
}

void PipeServer::run(int clientsCount)
{
    for (int i = 0; i < clientsCount; ++i)
    {
        HANDLE pipe = CreateNamedPipe(
            PIPE_NAME,
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE |
            PIPE_READMODE_MESSAGE |
            PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            sizeof(Response),
            sizeof(Request),
            0,
            nullptr
        );

        ConnectNamedPipe(pipe, nullptr);

        processClient(pipe);
    }
}