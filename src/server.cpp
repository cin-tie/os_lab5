#include <iostream>
#include <vector>
#include <windows.h>

#include "../include/file_manager.h"
#include "../include/lock_manager.h"
#include "../include/pipe_server.h"

void printFile(FileManager& fm)
{
    int count = fm.getRecordCount();

    for (int i = 0; i < count; ++i)
    {
        employee e = fm.readRecord(i);

        std::cout
            << e.num << " "
            << e.name << " "
            << e.hours << "\n";
    }
}

int main()
{
    std::string filename;
    int count;

    std::cin >> filename;
    std::cin >> count;

    std::vector<employee> records(count);

    for (int i = 0; i < count; ++i)
    {
        std::cin
            >> records[i].num
            >> records[i].name
            >> records[i].hours;
    }

    FileManager fileManager(filename);
    fileManager.initialize(records);

    printFile(fileManager);

    LockManager lockManager(count);

    int clientsCount;
    std::cin >> clientsCount;

    for (int i = 0; i < clientsCount; ++i)
    {
        STARTUPINFO si{};
        PROCESS_INFORMATION pi{};

        si.cb = sizeof(si);

        CreateProcess(
            "client.exe",
            nullptr,
            nullptr,
            nullptr,
            FALSE,
            CREATE_NEW_CONSOLE,
            nullptr,
            nullptr,
            &si,
            &pi
        );

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    PipeServer server(fileManager, lockManager);

    server.run(clientsCount);

    
    std::string cmd = "";
    while(cmd != "exit"){
        std::cin >> cmd;
        printFile(fileManager);
    }
    return 0;
}