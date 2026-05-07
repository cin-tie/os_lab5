#pragma once

#include "employee.h"

// Command types
enum class CommandType
{
    READ_LOCK,
    READ_RELEASE,

    WRITE_LOCK,
    WRITE_COMMIT,
    WRITE_RELEASE,

    EXIT
};

// Request structure for pipes
struct Request
{
    CommandType type;
    int recordId;
    employee data;
};

// Response structure for pipes
struct Response
{
    bool success;
    employee data;
};