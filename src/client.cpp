#include "../include/protocol.h"
#include "../include/utils.h"
#include <windows.h>
#include <iostream>

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

            std::cout << "1-READ 2-WRITE 3-EXIT: ";

            std::cin >> command;

            if (command < 1 || command > 3) {
                std::cout << "Error: Command must be 1, 2, or 3\n";
                continue;
            }

            Request request{};

            if (command == 1) {
                request.type = CommandType::READ;

                std::cout << "Record ID: ";
                std::cin >> request.recordId;

                if (request.recordId < 0) {
                    std::cout << "Error: Record ID must be >= 0\n";
                    continue;
                }
            }
            else if (command == 2) {
                request.type = CommandType::WRITE;

                std::cout << "Record ID: ";
                std::cin >> request.recordId;

                if (request.recordId < 0) {
                    std::cout << "Error: Record ID must be >= 0\n";
                    continue;
                }

                std::cout << "num: ";
                std::cin >> request.data.num;

                std::cout << "name: ";
                std::cin >> request.data.name;

                std::cout << "hours: ";
                std::cin >> request.data.hours;

                if (request.data.hours < 0) {
                    std::cout << "Error: Hours must be >= 0\n";
                    continue;
                }
            }
            else {
                request.type = CommandType::EXIT;
            }

            DWORD bytesWritten;

            if (!WriteFile(
                hPipe,
                &request,
                sizeof(request),
                &bytesWritten,
                nullptr
            )) {
                std::cout << "Error: Failed to send request\n";
                break;
            }

            if (request.type == CommandType::EXIT) {
                break;
            }

            Response response{};
            DWORD bytesRead;

            if (!ReadFile(
                hPipe,
                &response,
                sizeof(response),
                &bytesRead,
                nullptr
            )) {
                std::cout << "Error: Failed to read response\n";
                break;
            }

            if (!response.success) {
                std::cout << "Operation failed (invalid record ID?)\n";
                continue;
            }

            if (request.type == CommandType::READ) {
                std::cout << response.data.num
                          << " "
                          << response.data.name
                          << " "
                          << response.data.hours
                          << std::endl;
            }
            else if (request.type == CommandType::WRITE) {
                std::cout << "Write successful\n";
            }
        }

        CloseHandle(hPipe);
    }
    catch (const std::exception& ex) {
        std::cout << ex.what() << std::endl;
        return 1;
    }

    return 0;
}