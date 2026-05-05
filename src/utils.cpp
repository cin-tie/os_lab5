#include "../include/utils.h"

void GetLastError(const std::string& msg){
    throw std::runtime_error(
        msg + "Error: " + std::to_string(GetLastError())
    );
}