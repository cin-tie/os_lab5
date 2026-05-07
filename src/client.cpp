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
                break;
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
                break;
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

                if (s == "exit")
                {
                    std::cout << "Commit changes(y/n): ";
                    std::string c;
                    std::cin >> c;

                    if (c == "y" || c == "Y")
                    {
                        req.type = CommandType::WRITE_COMMIT;
                        req.data = e;
                        WriteFile(hPipe, &req, sizeof(req), &bytes, nullptr);
                        ReadFile(hPipe, &res, sizeof(res), &bytes, nullptr);
                    }

                    break;
                }

                if (s == "commit")
                {
                    req.type = CommandType::WRITE_COMMIT;
                    req.data = e;
                    WriteFile(hPipe, &req, sizeof(req), &bytes, nullptr);
                    ReadFile(hPipe, &res, sizeof(res), &bytes, nullptr);
                    e = res.data;
                }

                if (s == "num")
                {
                    std::cout << "Enter number: ";

                    int temp;
                    std::cin >> temp;

                    if (std::cin.fail())
                    {
                        std::cin.clear();
                        std::cin.ignore(10000, '\n');
                        std::cout << "Invalid\n";
                    }
                    else
                    {
                        e.num = temp;
                    }
                }

                if (s == "name")
                {
                    std::cout << "Enter name: ";

                    char temp[NAME_SIZE];
                    std::cin >> temp;

                    if (std::cin.fail())
                    {
                        std::cin.clear();
                        std::cin.ignore(10000, '\n');
                        std::cout << "Invalid\n";
                    }
                    else
                    {
                        strncpy(e.name, temp, NAME_SIZE - 1);
                        e.name[NAME_SIZE - 1] = '\0';
                    }
                }

                if (s == "hours")
                {
                    std::cout << "Enter hours: ";

                    double temp;
                    std::cin >> temp;

                    if (std::cin.fail() || temp < 0)
                    {
                        std::cin.clear();
                        std::cin.ignore(10000, '\n');
                        std::cout << "Invalid\n";
                    }
                    else
                    {
                        e.hours = temp;
                    }
                }

                std::cout << "\nEmployee #" << id << "\n";
                std::cout << e.num << "\n";
                std::cout << e.name << "\n";
                std::cout << e.hours << "\n";
            }

            req.type = CommandType::WRITE_RELEASE;
            WriteFile(hPipe, &req, sizeof(req), &bytes, nullptr);
        }
    }

    CloseHandle(hPipe);
}