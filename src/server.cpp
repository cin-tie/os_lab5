#include <iostream>
#include <vector>
#include <windows.h>
#include <string>

#include "../include/file_manager.h"
#include "../include/lock_manager.h"
#include "../include/pipe_server.h"
#include "../include/utils.h"

// Print binary file
void printFile(FileManager& fileManager) {
    try {
        int count = fileManager.getRecordCount();
        
        std::cout << "\n========== Current File Contents ==========" << std::endl;
        for (int i = 0; i < count; ++i) {
            employee e = fileManager.readRecord(i);
            std::cout << "Record " << i << ": "
                      << e.num << " | "
                      << e.name << " | "
                      << e.hours << std::endl;
        }
        std::cout << "============================================" << std::endl;
    } catch (const std::exception& e) {
        PrintError("Failed to print file contents: " + std::string(e.what()));
    }
}

int main()
{
    std::cout << "=== Server Application ===" << std::endl;

    try{

        // Get filename and record count
        std::string filename;
        int recordCount;
        
        std::cout << "Enter filename: ";
        std::getline(std::cin, filename);
        
        if (filename.empty()) {
            filename = "employees.dat";
            std::cout << "Using default filename: " << filename << std::endl;
        }
        
        recordCount = GetValidIntInput("Enter number of employees: ", 1, 1000);

        std::vector<employee> records(recordCount);

        // Get employees
        std::cout << "\nEnter employee data:" << std::endl;
        for (int i = 0; i < recordCount; ++i) {
            std::cout << "\n--- Employee #" << i << " ---" << std::endl;
            
            records[i].num = GetValidIntInput("ID number: ", 1, 999999);
            
            std::string name = GetValidStringInput("Name (max 9 chars): ", 9);
            strncpy(records[i].name, name.c_str(), 9);
            records[i].name[9] = '\0';
            
            records[i].hours = GetValidDoubleInput("Hours worked: ", 0);
        }

        // Init file and lock managers
        std::cout << "\nCreating file..." << std::endl;
        FileManager fileManager(filename);
        fileManager.initialize(records);

        std::cout << "\nFile created successfully!" << std::endl;
        printFile(fileManager);
        
        LockManager lockManager(recordCount);
        
        // Get client count
        int clientsCount = GetValidIntInput("\nEnter number of clients to start: ", 1, 50);

        std::cout << "\nStarting " << clientsCount << " client(s)..." << std::endl;

        // Start client processes
        for (int i = 0; i < clientsCount; ++i) {
            STARTUPINFO si{};
            PROCESS_INFORMATION pi{};
            
            si.cb = sizeof(si);
            
            std::string cmdLine = "client.exe";
            
            if (!CreateProcess(
                nullptr,
                const_cast<char*>(cmdLine.c_str()),
                nullptr,
                nullptr,
                FALSE,
                CREATE_NEW_CONSOLE,
                nullptr,
                nullptr,
                &si,
                &pi
            )) {
                PrintError("Failed to start client " + std::to_string(i + 1));
                continue;
            }
            
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            std::cout << "Client " << (i + 1) << " started" << std::endl;
        }

        std::cout << "\nStarting pipe server..." << std::endl;
        PipeServer server(fileManager, lockManager);
        server.run(clientsCount);


        std::cout << "\nPress any button to see file content. Type 'exit' to shutdown. \n";
        std::string cmd;
        while (std::getline(std::cin, cmd)) {
            if (cmd == "exit" || cmd == "quit") {
                break;
            }
            printFile(fileManager);
            std::cout << "Type 'exit' to shutdown: ";
        }

    } catch (const std::exception& e){
        PrintError("Server error: " + std::string(e.what()));
        std::cout << "Press Enter to exit..." << std::endl;
        std::cin.get();
        return 1;
    }

    std::cout << "Server shutting down..." << std::endl;
    return 0;
}