#include "../include/utils.h"

void ThrowLastError(const std::string& msg){
    std::cout << msg << " Error code: " << std::to_string(::GetLastError());
}