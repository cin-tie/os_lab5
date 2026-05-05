#include "../include/file_manager.h"
#include "../include/lock_manager.h"
#include "../include/pipe_server.h"
#include "../include/utils.h"
#include <windows.h>
#include <iostream>
#include <vector>

int main() {
    try {
        std::string filename;
        int recordCount;

        std::cout << "Enter file name: ";
        std::cin >> filename;

        std::cout << "Enter record count: ";
        std::cin >> recordCount;

        std::vector<employee> employees(recordCount);

        for (int i = 0; i < recordCount; ++i) {
            std::cout << "Employee #" << i << "\n";

            std::cout << "num: ";
            std::cin >> employees[i].num;

            std::cout << "name: ";
            std::cin >> employees[i].name;

            std::cout << "hours: ";
            std::cin >> employees[i].hours;
        }

        FileManager fileManager(filename);
        fileManager.initialize(employees);

        LockManager lockManager(recordCount);

        int clientCount;

        std::cout << "Enter client count: ";
        std::cin >> clientCount;

        for (int i = 0; i < clientCount; ++i) {
            STARTUPINFO si{};
            PROCESS_INFORMATION pi{};

            si.cb = sizeof(si);

            char command[] = "client.exe";

            if (!CreateProcess(
                nullptr,
                command,
                nullptr,
                nullptr,
                FALSE,
                CREATE_NEW_CONSOLE,
                nullptr,
                nullptr,
                &si,
                &pi
            )) {
                ThrowLastError("CreateProcess failed");
            }

            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }

        PipeServer server(
            fileManager,
            lockManager
        );

        server.run(clientCount);
    }
    catch (const std::exception& ex) {
        std::cout << ex.what() << std::endl;
        return 1;
    }

    return 0;
}