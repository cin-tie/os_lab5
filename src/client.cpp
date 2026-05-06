#include <windows.h>
#include <iostream>
#include "../include/protocol.h"

const char* PIPE_NAME = "\\\\.\\pipe\\lab5_pipe";

int main(){
    HANDLE hPipe = CreateFile(
        PIPE_NAME,
        GENERIC_READ | GENERIC_WRITE,
        0, nullptr,
        OPEN_EXISTING,
        0, nullptr
    );

    if(hPipe == INVALID_HANDLE_VALUE){
        return 1;
    }

    Request req;
    Response res;
    DWORD bytes;

    std::cout << "Client started!" << std::endl;
    while(true){
        int cmd;
        std::cout << "\nChoose menu option:\n1-read 2-write 0-exit" << std::endl;
        std::cin >> cmd;

        if(cmd != 0 && cmd != 1 && cmd != 2){
            std::cout << "\nUnknown function..." << std::endl;
            continue;
        }

        if(cmd == 0){
            req.type = CommandType::EXIT;
            WriteFile(hPipe, &req, sizeof(req), &bytes, nullptr);
            break;
        }

        if(cmd == 1){
            int id;
            std::cout << "\nEnter id to read: ";
            std::cin >> id;
            
            req.type = CommandType::READ_LOCK;
            req.recordId = id;

            WriteFile(hPipe, &req, sizeof(req), &bytes, nullptr);
            ReadFile(hPipe, &res, sizeof(res), &bytes, nullptr);

            if(!res.success){
                std::cout << "Something went wrong with reading..." << std::endl;
                continue;
            }

            std::cout << "\tEmployee #" + std::to_string(id) << std::endl;
            std::cout << "Num:\t" << res.data.num << "\nName:\t"
                      << res.data.name << "\nHours:\t "
                      << res.data.hours << "\n";

            std::string s;
            std::cout << "Press any button to read\nWrite \"exit\" to leave reading" << std::endl;
            while (true)
            {
                std::cin >> s;
                if (s == "exit") {
                    break;
                }
                
                std::cout << "\n\tEmployee #" + std::to_string(id) << std::endl;
                std::cout << "Num:\t" << res.data.num << "\nName:\t"
                        << res.data.name << "\nHours:\t "
                        << res.data.hours << "\n";
            }
            req.type = CommandType::READ_RELEASE;
            WriteFile(hPipe, &req, sizeof(req), &bytes, nullptr);
        }

        if(cmd == 2){
            int id;
            std::cout << "\nEnter id to write: ";
            std::cin >> id;

            req.type = CommandType::WRITE_LOCK;
            req.recordId = id;

            WriteFile(hPipe, &req, sizeof(req), &bytes, nullptr);
            ReadFile(hPipe, &res, sizeof(res), &bytes, nullptr);

            if(!res.success){
                std::cout << "Something went wrong with writing..." << std::endl;
                continue;
            }

            std::cout << "\tEmployee #" + std::to_string(id) << std::endl;
            std::cout << "Num:\t" << res.data.num << "\nName:\t"
                      << res.data.name << "\nHours:\t "
                      << res.data.hours << "\n";

            employee e = res.data;

            std::string s;
            std::cout << "Write \"num\" to change number\nWrite \"name\" to change name\nWrite \"hours\" to change hours\nWrite \"commit\" to upload changes" << std::endl;
            std::cout << "Write \"exit\" to leave reading" << std::endl;
            while (true)
            {
                std::cin >> s;
                if (s == "exit"){
                    std::cout << "Commit changes(y/n): ";
                    std::string c;
                    std::cin >> c;
                    if(c == "y" || c == "Y"){
                        req.type = CommandType::WRITE_COMMIT;
                        req.data = e;
                        WriteFile(hPipe, &req, sizeof(req), &bytes, nullptr);
                    }
                    break;
                }
                if(s == "commit"){
                    req.type = CommandType::WRITE_COMMIT;
                    req.data = e;
                    WriteFile(hPipe, &req, sizeof(req), &bytes, nullptr);
                }
                if(s == "num"){
                    std::cout << "Enter number: ";
                    int temp = e.num;
                    std::cin >> e.num;
                    if (std::cin.fail()) {
                        std::cerr << "Error: Invalid number input!" << std::endl;
                        e.num = temp;
                    }
                }
                if(s == "name"){
                    std::cout << "Enter name: ";
                    int temp = e.num;
                    std::cin >> e.name;
                    if (std::cin.fail()) {
                        std::cerr << "Error: Invalid name input!" << std::endl;
                        e.num = temp;
                    }
                }
                if(s == "hours"){
                    std::cout << "Enter hours: ";
                    int temp = e.num;
                    std::cin >> e.hours;
                    if (std::cin.fail() || e.hours < 0) {
                        std::cerr << "Error: Hours must be a non-negative number!" << std::endl;
                        e.num = temp;
                    }
                }

                std::cout << "\n\tEmployee #" + std::to_string(id) << std::endl;
                std::cout << "Num:\t" << res.data.num << "\nName:\t"
                        << res.data.name << "\nHours:\t "
                        << res.data.hours << "\n";
            }

            req.type = CommandType::WRITE_RELEASE;
            WriteFile(hPipe, &req, sizeof(req), &bytes, nullptr);
        }
    }

    CloseHandle(hPipe);
}