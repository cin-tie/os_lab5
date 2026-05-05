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

            std::cout
                << "1-READ 2-WRITE 3-EXIT: ";

            std::cin >> command;

            Request request{};

            if (command == 1) {
                request.type =
                    CommandType::READ;

                std::cout
                    << "Record ID: ";

                std::cin
                    >> request.recordId;
            }
            else if (command == 2) {
                request.type =
                    CommandType::WRITE;

                std::cout
                    << "Record ID: ";

                std::cin
                    >> request.recordId;

                std::cout
                    << "num: ";

                std::cin
                    >> request.data.num;

                std::cout
                    << "name: ";

                std::cin
                    >> request.data.name;

                std::cout
                    << "hours: ";

                std::cin
                    >> request.data.hours;
            }
            else {
                request.type =
                    CommandType::EXIT;
            }

            DWORD bytesWritten;

            WriteFile(
                hPipe,
                &request,
                sizeof(request),
                &bytesWritten,
                nullptr
            );

            if (request.type ==
                CommandType::EXIT) {
                break;
            }

            Response response{};
            DWORD bytesRead;

            ReadFile(
                hPipe,
                &response,
                sizeof(response),
                &bytesRead,
                nullptr
            );

            if (!response.success) {
                std::cout
                    << "Operation failed\n";
                continue;
            }

            if (request.type ==
                CommandType::READ) {
                std::cout
                    << response.data.num
                    << " "
                    << response.data.name
                    << " "
                    << response.data.hours
                    << std::endl;
            }
        }

        CloseHandle(hPipe);
    }
    catch (const std::exception& ex) {
        std::cout
            << ex.what()
            << std::endl;

        return 1;
    }

    return 0;
}