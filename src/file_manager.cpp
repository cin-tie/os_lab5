#include "../include/file_manager.h"
#include "../include/utils.h"

// Creation of file
FileManager::FileManager(const std::string& filename){
    fileHandle = CreateFile(
        filename.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if(fileHandle == INVALID_HANDLE_VALUE)
        ThrowLastError("CreateFile failed");
}

// Initialization with records
void FileManager::initialize(const std::vector<employee>& records){
    DWORD written;

    SetFilePointer(fileHandle, 0, nullptr, FILE_BEGIN);

    for(const auto& rec : records){
        if(!WriteFile(fileHandle, &rec, sizeof(employee), &written, nullptr)){
            ThrowLastError("WriteFile failed");
        }
    }

    FlushFileBuffers(fileHandle);
}

// Reading record from file
employee FileManager::readRecord(int index){
    employee emp;
    DWORD read;

    SetFilePointer(fileHandle, index * sizeof(employee), nullptr, FILE_BEGIN);

    if(!ReadFile(fileHandle, &emp, sizeof(employee), &read, nullptr)){
        ThrowLastError("ReadFile failed");
    }

    return emp;
}

// Writing record to file
void FileManager::writeRecord(int index, const employee& emp){
    DWORD written;

    SetFilePointer(fileHandle, index * sizeof(employee), nullptr, FILE_BEGIN);

    if(!WriteFile(fileHandle, &emp, sizeof(employee), &written, nullptr)){
        ThrowLastError("WriteFile failed");
    }

    FlushFileBuffers(fileHandle);
}

// Record count
int FileManager::getRecordCount(){
    DWORD size = GetFileSize(fileHandle, nullptr);
    return size / sizeof(employee);
}

// Destructor
FileManager::~FileManager(){
    CloseHandle(fileHandle);
}