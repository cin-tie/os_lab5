#include "../include/utils.h"

void ThrowLastError(const std::string& msg) {
    DWORD error = GetLastError();
    std::string fullMsg = msg + " Error code: " + std::to_string(error);
    throw std::runtime_error(fullMsg);
}

void ThrowIfFailed(BOOL result, const std::string& msg) {
    if (!result) {
        ThrowLastError(msg);
    }
}

void PrintError(const std::string& msg) {
    std::cerr << "[ERROR] " << msg << std::endl;
}

bool IsValidEmployee(const employee& emp) {
    return emp.num > 0 && emp.name[0] != '\0' && emp.hours >= 0;
}

int GetValidIntInput(const std::string& prompt, int min, int max) {
    int value;
    while (true) {
        std::cout << prompt;
        std::cin >> value;
        
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            PrintError("Invalid input. Please enter a number.");
        } else if (value < min || value > max) {
            PrintError("Value out of range. Expected " + std::to_string(min) + " to " + std::to_string(max));
        } else {
            std::cin.ignore(10000, '\n');
            return value;
        }
    }
}

double GetValidDoubleInput(const std::string& prompt, double min) {
    double value;
    while (true) {
        std::cout << prompt;
        std::cin >> value;
        
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            PrintError("Invalid input. Please enter a number.");
        } else if (value < min) {
            PrintError("Hours cannot be negative.");
        } else {
            std::cin.ignore(10000, '\n');
            return value;
        }
    }
}

std::string GetValidStringInput(const std::string& prompt, size_t maxLen) {
    std::string value;
    while (true) {
        std::cout << prompt;
        std::getline(std::cin, value);
        
        if (value.empty()) {
            PrintError("Name cannot be empty.");
        } else if (value.length() > maxLen) {
            PrintError("Name too long. Max " + std::to_string(maxLen) + " characters.");
        } else {
            return value;
        }
    }
}