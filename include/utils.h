#pragma once

#include <windows.h>
#include <string>
#include <stdexcept>

void ThrowLastError(const std::string& msg);