#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include "employee.h"

// File manager
class FileManager
{
private:
    HANDLE fileHandle;

public:
    FileManager(const std::string& filename);

    void initialize(const std::vector<employee>& records);

    employee readRecord(int index);

    void writeRecord(int index, const employee& emp);

    int getRecordCount();

    ~FileManager();
};