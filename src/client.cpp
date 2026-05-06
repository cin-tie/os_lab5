#include "../include/protocol.h"
#include "../include/utils.h"
#include <windows.h>
#include <iostream>
#include <string>

const char* PIPE_NAME = "\\\\.\\pipe\\lab5_pipe";

int main() {
    try {
        if (!WaitNamedPipe(
            PIPE_NAME,
            NMPWAIT_WAIT_FOREVER
        )) {
            ThrowLastError(
                "WaitNamedPipe failed"
            );
        }

        HANDLE hPipe = CreateFile(
            PIPE_NAME,
            GENERIC_READ |
            GENERIC_WRITE,
            0,
            nullptr,
            OPEN_EXISTING,
            0,
            nullptr
        );

        if (hPipe ==
            INVALID_HANDLE_VALUE) {
            ThrowLastError(
                "CreateFile failed"
            );
        }

        while (true) {
            int command;

            std::cout << "\n===== MENU =====" << std::endl;
            std::cout << "1 - READ record" << std::endl;
            std::cout << "2 - MODIFY record" << std::endl;
            std::cout << "3 - EXIT" << std::endl;
            std::cout << "Choice: ";

            std::cin >> command;

            if (command < 1 || command > 3) {
                std::cout << "Error: Command must be 1, 2, or 3\n";
                continue;
            }

            if (command == 1) {
                Request request{};
                request.type = CommandType::READ;

                std::cout << "Enter record ID to read: ";
                std::cin >> request.recordId;

                if (request.recordId < 0) {
                    std::cout << "Error: Record ID must be >= 0\n";
                    continue;
                }

                DWORD bytesWritten;
                if (!WriteFile(hPipe, &request, sizeof(request), &bytesWritten, nullptr)) {
                    std::cout << "Error: Failed to send read request\n";
                    break;
                }

                Response response{};
                DWORD bytesRead;
                if (!ReadFile(hPipe, &response, sizeof(response), &bytesRead, nullptr)) {
                    std::cout << "Error: Failed to read response\n";
                    break;
                }

                if (!response.success) {
                    std::cout << "Operation failed (invalid record ID?)\n";
                    continue;
                }

                std::cout << "\n=== RECORD READ ===" << std::endl;
                std::cout << "ID:   " << response.data.num << std::endl;
                std::cout << "Name: " << response.data.name << std::endl;
                std::cout << "Hours: " << response.data.hours << std::endl;
                std::cout << "===================\n" << std::endl;
            }
            else if (command == 2) {
                Request request{};
                request.type = CommandType::READ;

                std::cout << "Enter record ID to modify: ";
                std::cin >> request.recordId;

                if (request.recordId < 0) {
                    std::cout << "Error: Record ID must be >= 0\n";
                    continue;
                }

                DWORD bytesWritten;
                if (!WriteFile(hPipe, &request, sizeof(request), &bytesWritten, nullptr)) {
                    std::cout << "Error: Failed to send read request\n";
                    break;
                }

                Response response{};
                DWORD bytesRead;
                if (!ReadFile(hPipe, &response, sizeof(response), &bytesRead, nullptr)) {
                    std::cout << "Error: Failed to read response\n";
                    break;
                }

                if (!response.success) {
                    std::cout << "Operation failed (invalid record ID?)\n";
                    continue;
                }

                std::cout << "\n=== CURRENT RECORD ===" << std::endl;
                std::cout << "ID:   " << response.data.num << std::endl;
                std::cout << "Name: " << response.data.name << std::endl;
                std::cout << "Hours: " << response.data.hours << std::endl;
                std::cout << "======================\n" << std::endl;

                employee modifiedData;
                std::cout << "Enter NEW values:" << std::endl;
                std::cout << "New num: ";
                std::cin >> modifiedData.num;
                
                std::cout << "New name: ";
                std::cin >> modifiedData.name;
                
                std::cout << "New hours: ";
                std::cin >> modifiedData.hours;

                if (modifiedData.hours < 0) {
                    std::cout << "Error: Hours must be >= 0\n";
                    continue;
                }

                std::cout << "\nDo you want to save changes? (y/n): ";
                std::string confirm;
                std::cin >> confirm;
                
                if (confirm != "y" && confirm != "Y") {
                    std::cout << "Modification cancelled.\n";
                    continue;
                }

                Request writeRequest{};
                writeRequest.type = CommandType::WRITE;
                writeRequest.recordId = request.recordId;
                writeRequest.data = modifiedData;

                if (!WriteFile(hPipe, &writeRequest, sizeof(writeRequest), &bytesWritten, nullptr)) {
                    std::cout << "Error: Failed to send write request\n";
                    break;
                }

                Response writeResponse{};
                if (!ReadFile(hPipe, &writeResponse, sizeof(writeResponse), &bytesRead, nullptr)) {
                    std::cout << "Error: Failed to read write response\n";
                    break;
                }

                if (writeResponse.success) {
                    std::cout << "Write successful! Record modified.\n";
                } else {
                    std::cout << "Write failed!\n";
                }
            }
            else {
                Request request{};
                request.type = CommandType::EXIT;
                
                DWORD bytesWritten;
                WriteFile(hPipe, &request, sizeof(request), &bytesWritten, nullptr);
                break;
            }
        }

        CloseHandle(hPipe);
        std::cout << "\nDisconnected from server.\n";
    }
    catch (const std::exception& ex) {
        std::cout << ex.what() << std::endl;
        return 1;
    }

    return 0;
}