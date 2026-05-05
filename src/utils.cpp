#include "../include/utils.h"

void ThrowLastError(const std::string& msg){
    throw std::runtime_error(msg + " Error code: " + std::to_string(::GetLastError()));
}