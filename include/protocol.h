#pragma once

#include "employee.h"

enum class CommandType
{
    READ_LOCK,
    WRITE_LOCK,
    UPDATE,
    UNLOCK,
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
