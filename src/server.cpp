#include "../include/file_manager.h"
#include "../include/lock_manager.h"
#include "../include/pipe_server.h"
#include "../include/utils.h"
#include <windows.h>
#include <iostream>
#include <vector>
#include <limits>

int main() {
    try {
        std::string filename;
        int recordCount;

        std::cout << "Enter file name: ";
        std::cin >> filename;
        
        if (filename.empty()) {
            std::cerr << "Error: File name cannot be empty!" << std::endl;
            return 1;
        }

        std::cout << "Enter record count: ";
        std::cin >> recordCount;
        
        if (std::cin.fail() || recordCount <= 0) {
            std::cerr << "Error: Record count must be a positive number!" << std::endl;
            return 1;
        }

        std::vector<employee> employees(recordCount);

        for (int i = 0; i < recordCount; ++i) {
            std::cout << "Employee #" << i << "\n";

            std::cout << "num: ";
            std::cin >> employees[i].num;
            if (std::cin.fail()) {
                std::cerr << "Error: Invalid number input!" << std::endl;
                return 1;
            }

            std::cout << "name: ";
            std::cin >> employees[i].name;
            if (std::cin.fail()) {
                std::cerr << "Error: Invalid name input!" << std::endl;
                return 1;
            }

            std::cout << "hours: ";
            std::cin >> employees[i].hours;
            if (std::cin.fail() || employees[i].hours < 0) {
                std::cerr << "Error: Hours must be a non-negative number!" << std::endl;
                return 1;
            }
        }

        FileManager fileManager(filename);
        fileManager.initialize(employees);
        
        // 1.2. Выводит созданный файл на консоль
        std::cout << "\n=== CREATED FILE ===" << std::endl;
        for (int i = 0; i < recordCount; ++i) {
            employee emp = fileManager.readRecord(i);
            std::cout << emp.num << " " << emp.name << " " << emp.hours << std::endl;
        }
        std::cout << "===================\n" << std::endl;

        LockManager lockManager(recordCount);

        int clientCount;

        std::cout << "Enter client count: ";
        std::cin >> clientCount;
        
        if (std::cin.fail() || clientCount <= 0) {
            std::cerr << "Error: Client count must be a positive number!" << std::endl;
            return 1;
        }

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
                std::cerr << "Warning: Failed to create client process #" << i << std::endl;
            } else {
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
            }
        }

        PipeServer server(fileManager, lockManager);
        server.run(clientCount);
        
        // 1.5. После завершения работы всех процессов клиентов выводит на консоль модифицированный файл
        std::cout << "\n=== MODIFIED FILE ===" << std::endl;
        for (int i = 0; i < recordCount; ++i) {
            employee emp = fileManager.readRecord(i);
            std::cout << emp.num << " " << emp.name << " " << emp.hours << std::endl;
        }
        std::cout << "====================\n" << std::endl;
        
        // 1.6. По команде с консоли завершает свою работу
        std::cout << "Press Enter to exit...";
        std::cin.ignore();
        std::cin.get();
    }
    catch (const std::exception& ex) {
        std::cout << ex.what() << std::endl;
        std::cout << "Press Enter to exit...";
        std::cin.ignore();
        std::cin.get();
        return 1;
    }

    return 0;
}