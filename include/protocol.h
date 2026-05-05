#pragma once

#include "employee.h"

enum class CommandType{
    READ,
    WRITE,
    EXIT
};

struct Request
{
    CommandType type;
    int recordId;
    employee data;
};

struct Response
{
    bool success;
    employee data;
};
