#include <windows.h>
#include <iostream>
#include <string>
#include "../include/protocol.h"
#include "../include/utils.h"

const char* PIPE_NAME = "\\\\.\\pipe\\lab5_pipe";

// Connect to server with retries
HANDLE ConnectToServer(int maxRetries = 10){
    HANDLE hPipe;
    int retries = 0;

    while(retries < maxRetries){
        // Create connection
        hPipe = CreateFile(
            PIPE_NAME,
            GENERIC_READ | GENERIC_WRITE,
            0, nullptr,
            OPEN_EXISTING,
            0, nullptr
        );

        if(hPipe != INVALID_HANDLE_VALUE){
            return hPipe;
        }

        DWORD error = GetLastError();
        if (error == ERROR_PIPE_BUSY) {
            std::cout << "Server is busy, waiting..." << std::endl;
            Sleep(1000);
            retries++;
        } else {
            PrintError("Failed to connect to server. Make sure server is running.");
            return INVALID_HANDLE_VALUE;
        }
    }

    PrintError("Max retries reached. Could not connect to server.");
    return INVALID_HANDLE_VALUE;
}

// Send request
bool SendRequest(HANDLE hPipe, Request& req, Response& res){
    DWORD bytes;

    if (!WriteFile(hPipe, &req, sizeof(req), &bytes, nullptr)) {
        PrintError("Failed to send request to server");
        return false;
    }
    
    if (!ReadFile(hPipe, &res, sizeof(res), &bytes, nullptr)) {
        PrintError("Failed to receive response from server");
        return false;
    }
    
    return true;
}

// Employee
void DisplayEmployee(const employee& emp) {
    std::cout << "\n========== Employee Info ==========" << std::endl;
    std::cout << "Number: " << emp.num << std::endl;
    std::cout << "Name:   " << emp.name << std::endl;
    std::cout << "Hours:  " << emp.hours << std::endl;
    std::cout << "===================================" << std::endl;
}

// Menu
void DisplayMenu(){
    std::cout << "\n========== Menu ==========" << std::endl;
    std::cout << "1 - Read employee record" << std::endl;
    std::cout << "2 - Modify employee record" << std::endl;
    std::cout << "0 - Exit" << std::endl;
    std::cout << "==========================" << std::endl;
}

// Edit menu
void DisplayEditMenu(){
    std::cout << "\n===== Edit Menu =====" << std::endl;
    std::cout << "1 - Modify number" << std::endl;
    std::cout << "2 - Modify name" << std::endl;
    std::cout << "3 - Modify hours" << std::endl;
    std::cout << "4 - Commit changes" << std::endl;
    std::cout << "5 - Exit and release lock" << std::endl;
    std::cout << "====================" << std::endl;
}

int main(){
    std::cout << "=== Client Application ===" << std::endl;
    
    // Connecting
    HANDLE hPipe = ConnectToServer();
    if (hPipe == INVALID_HANDLE_VALUE) {
        std::cout << "Press Enter to exit..." << std::endl;
        std::cin.get();
        return 1;
    }

    std::cout << "Connected to server!" << std::endl;
    
    Request req;
    Response res;

    std::cout << "Client started!" << std::endl;
    // Main loop
    while(true){
        DisplayMenu();

        int cmd = GetValidIntInput("Choose option: ", 0, 2);

        // Exit
        if (cmd == 0) {
            req.type = CommandType::EXIT;
            req.recordId = -1;
            
            DWORD bytes;
            WriteFile(hPipe, &req, sizeof(req), &bytes, nullptr);
            std::cout << "Exiting..." << std::endl;
            break;
        }

        // Get id for working
        int recordCount;
        int index = GetValidIntInput("Enter employee ID (number): ");

        // READ
        if(cmd == 1){
            // Read lock
            req.type = CommandType::READ_LOCK;
            req.recordId = index;

            if (!SendRequest(hPipe, req, res)) {
                continue;
            }

            if (!res.success) {
                PrintError("Failed to read record. Record may be locked or doesn't exist.");
                continue;
            }

            // Record
            std::cout << "\n*** Record read successfully ***" << std::endl;
            DisplayEmployee(res.data);
            std::cout << "\nPress any key to read again, or type 'exit' to release lock: ";

            // Show record loop
            std::string input;
            while (true) {
                std::getline(std::cin, input);
                if (input == "exit" || input == "q") {
                    break;
                }
                DisplayEmployee(res.data);
                std::cout << "\nPress any key to read again, or type 'exit' to release lock: ";
            }

            // Release read lock
            req.type = CommandType::READ_RELEASE;
            req.recordId = index;
            DWORD bytes;
            WriteFile(hPipe, &req, sizeof(req), &bytes, nullptr);
            std::cout << "Read lock released." << std::endl;
        }
        // WRITE
        if(cmd == 2){
            // Write lock
            req.type = CommandType::WRITE_LOCK;
            req.recordId = index;
            
            if(!SendRequest(hPipe, req, res)) {
                continue;
            }
            
            if(!res.success) {
                PrintError("Failed to acquire write lock. Record is being read or modified by another client.");
                std::cout << "Please try again later." << std::endl;
                continue;
            }

            // Employee for editing
            std::cout << "\n*** Write lock acquired ***" << std::endl;
            std::cout << "Current record:" << std::endl;
            DisplayEmployee(res.data);

            employee modified = res.data;
            bool modifiedFlag = false;

            // Edit loop
            while(true){
                DisplayEditMenu();

                int choice = GetValidIntInput("Choose action: ", 1, 5);

                // Number
                if(choice == 1) {
                    int newNum = GetValidIntInput("Enter new employee number: ", 1, 999999);
                    modified.num = newNum;
                    modifiedFlag = true;
                    std::cout << "Number updated to: " << modified.num << std::endl;
                } 
                // Name
                else if(choice == 2) {
                    std::string newName = GetValidStringInput("Enter new name (max 9 chars): ", 9);
                    strncpy(modified.name, newName.c_str(), 9);
                    modified.name[9] = '\0';
                    modifiedFlag = true;
                    std::cout << "Name updated to: " << modified.name << std::endl;
                }
                // Hours
                else if(choice == 3) {
                    double newHours = GetValidDoubleInput("Enter new hours: ", 0);
                    modified.hours = newHours;
                    modifiedFlag = true;
                    std::cout << "Hours updated to: " << modified.hours << std::endl;
                }
                // Send to server
                else if(choice == 4){
                    if (!modifiedFlag) {
                        std::cout << "No changes to commit." << std::endl;
                        continue;
                    }

                    req.type = CommandType::WRITE_COMMIT;
                    req.recordId = index;
                    req.data = modified;

                    if (SendRequest(hPipe, req, res)) {
                        if (res.success) {
                            std::cout << "\n*** Changes committed successfully! ***" << std::endl;
                            DisplayEmployee(res.data);
                        } else {
                            PrintError("Failed to commit changes");
                        }
                    }
                    
                    modifiedFlag = false;
                }
                // exit
                else if (choice == 5){
                    if(modifiedFlag){
                        std::cout << "You have uncommited changes!\nAre you sure to continue?(y|N): ";
                        std::string confirmation;
                        std::getline(std::cin, confirmation);
                        if(confirmation == "y" || confirmation == "Y"){
                            break;
                        }
                        continue;
                    }
                    std::cout << "Exiting and releasing lock..." << std::endl;
                    break;
                }
                if (modifiedFlag && choice != 4) {
                    std::cout << "\nCurrent modified record:" << std::endl;
                    DisplayEmployee(modified);
                    std::cout << "Use 'Commit changes' to save to server." << std::endl;
                }
            }
            // Release write lock
            req.type = CommandType::WRITE_RELEASE;
            req.recordId = index;
            DWORD bytes;
            WriteFile(hPipe, &req, sizeof(req), &bytes, nullptr);
            std::cout << "Write lock released." << std::endl;
        }
    }

    CloseHandle(hPipe);
    std::cout << "Disconnected from server." << std::endl;
    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();
    
    return 0;
}