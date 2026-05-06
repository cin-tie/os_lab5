#include "../include/protocol.h"
#include "../include/utils.h"
#include <windows.h>
#include <iostream>
#include <string>

const char* PIPE_NAME = "\\\\.\\pipe\\lab5_pipe";

int main() {
    try {
        if (!WaitNamedPipe(PIPE_NAME, NMPWAIT_WAIT_FOREVER)) {
            ThrowLastError("WaitNamedPipe failed");
        }

        HANDLE hPipe = CreateFile(
            PIPE_NAME,
            GENERIC_READ | GENERIC_WRITE,
            0,
            nullptr,
            OPEN_EXISTING,
            0,
            nullptr
        );

        if (hPipe == INVALID_HANDLE_VALUE) {
            ThrowLastError("CreateFile failed");
        }

        while (true) {
            int command;
            std::cout << "\n1-READ 2-MODIFY 3-EXIT: ";
            std::cin >> command;

            if (command < 1 || command > 3) {
                std::cout << "Invalid command\n";
                continue;
            }

            if (command == 1) {
                int recordId;
                std::cout << "Record ID: ";
                std::cin >> recordId;
                
                if (recordId < 0) {
                    std::cout << "Invalid ID\n";
                    continue;
                }

                Request req;
                req.type = CommandType::READ;
                req.recordId = recordId;
                
                DWORD bytesWritten;
                if (!WriteFile(hPipe, &req, sizeof(req), &bytesWritten, nullptr)) {
                    std::cout << "Write failed\n";
                    break;
                }

                Response resp;
                DWORD bytesRead;
                if (!ReadFile(hPipe, &resp, sizeof(resp), &bytesRead, nullptr)) {
                    std::cout << "Read failed\n";
                    break;
                }

                if (!resp.success) {
                    std::cout << "Record not found\n";
                    continue;
                }

                std::cout << "Record: " << resp.data.num << " " 
                          << resp.data.name << " " << resp.data.hours << std::endl;
                
            }
            else if (command == 2) {
                int recordId;
                std::cout << "Record ID: ";
                std::cin >> recordId;
                
                if (recordId < 0) {
                    std::cout << "Invalid ID\n";
                    continue;
                }

                Request req;
                req.type = CommandType::READ;
                req.recordId = recordId;
                
                DWORD bytesWritten;
                if (!WriteFile(hPipe, &req, sizeof(req), &bytesWritten, nullptr)) {
                    std::cout << "Write failed\n";
                    break;
                }

                Response resp;
                DWORD bytesRead;
                if (!ReadFile(hPipe, &resp, sizeof(resp), &bytesRead, nullptr)) {
                    std::cout << "Read failed\n";
                    break;
                }

                if (!resp.success) {
                    std::cout << "Record not found\n";
                    continue;
                }

                std::cout << "Current record: " << resp.data.num << " " 
                          << resp.data.name << " " << resp.data.hours << std::endl;
                
                employee newData;
                std::cout << "New num: ";
                std::cin >> newData.num;
                std::cout << "New name: ";
                std::cin >> newData.name;
                std::cout << "New hours: ";
                std::cin >> newData.hours;
                
                if (newData.hours < 0) {
                    std::cout << "Hours must be >= 0\n";
                    continue;
                }
                
                std::cout << "Send to server? (y/n): ";
                std::string confirm;
                std::cin >> confirm;
                
                if (confirm != "y" && confirm != "Y") {
                    std::cout << "Modification cancelled\n";
                    continue;
                }
                
                Request writeReq;
                writeReq.type = CommandType::WRITE;
                writeReq.recordId = recordId;
                writeReq.data = newData;
                
                if (!WriteFile(hPipe, &writeReq, sizeof(writeReq), &bytesWritten, nullptr)) {
                    std::cout << "Write failed\n";
                    break;
                }
                
                Response writeResp;
                if (!ReadFile(hPipe, &writeResp, sizeof(writeResp), &bytesRead, nullptr)) {
                    std::cout << "Read failed\n";
                    break;
                }
                
                if (writeResp.success) {
                    std::cout << "Record modified successfully\n";
                } else {
                    std::cout << "Modification failed\n";
                }
            }
            else if (command == 3) {
                Request req;
                req.type = CommandType::EXIT;
                DWORD bytesWritten;
                WriteFile(hPipe, &req, sizeof(req), &bytesWritten, nullptr);
                break;
            }
        }

        CloseHandle(hPipe);
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