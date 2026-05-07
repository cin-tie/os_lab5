#pragma once

#include "employee.h"
#include <windows.h>
#include <string>
#include <stdexcept>
#include <iostream>

void ThrowLastError(const std::string& msg);
void ThrowIfFailed(BOOL result, const std::string& msg);
void PrintError(const std::string& msg);
bool IsValidEmployee(const employee& emp);
int GetValidIntInput(const std::string& prompt, int min = 0, int max = 999999);
double GetValidDoubleInput(const std::string& prompt, double min = 0);
std::string GetValidStringInput(const std::string& prompt, size_t maxLen = 9);