#include "../include/file_manager.h"
#include "../include/utils.h"

// Creation of file
FileManager::FileManager(const std::string& filename){
    if (filename.empty()) {
        throw std::invalid_argument("Filename cannot be empty");
    }

    fileHandle = CreateFile(
        filename.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (fileHandle == INVALID_HANDLE_VALUE) {
        ThrowLastError("CreateFile failed for " + filename);
    }
}

// Initialization with records
void FileManager::initialize(const std::vector<employee>& records){
    if (records.empty()) {
        throw std::invalid_argument("Cannot initialize with empty records");
    }

    DWORD written;

    SetFilePointer(fileHandle, 0, nullptr, FILE_BEGIN);

    for (const auto& rec : records) {
        if (!IsValidEmployee(rec)) {
            throw std::invalid_argument("Invalid employee data found during initialization");
        }
        
        if (!WriteFile(fileHandle, &rec, sizeof(employee), &written, nullptr)) {
            ThrowLastError("WriteFile failed during initialization");
        }
        
        if (written != sizeof(employee)) {
            throw std::runtime_error("Incomplete write operation");
        }
    }
    
    if (!FlushFileBuffers(fileHandle)) {
        PrintError("Failed to flush file buffers");
    }
}

// Reading record from file
employee FileManager::readRecord(int index){
    if (index < 0) {
        throw std::out_of_range("Record index cannot be negative: " + std::to_string(index));
    }

    employee emp;
    DWORD read;
    
    DWORD result = SetFilePointer(fileHandle, index * sizeof(employee), nullptr, FILE_BEGIN);
    if (result == INVALID_SET_FILE_POINTER) {
        ThrowLastError("SetFilePointer failed for read at index " + std::to_string(index));
    }
    
    if (!ReadFile(fileHandle, &emp, sizeof(employee), &read, nullptr)) {
        ThrowLastError("ReadFile failed at index " + std::to_string(index));
    }
    
    if (read != sizeof(employee)) {
        throw std::runtime_error("Incomplete read operation at index " + std::to_string(index));
    }
    
    return emp;
}

// Writing record to file
void FileManager::writeRecord(int index, const employee& emp){
    if (index < 0) {
        throw std::out_of_range("Record index cannot be negative: " + std::to_string(index));
    }
    
    if (!IsValidEmployee(emp)) {
        throw std::invalid_argument("Invalid employee data - all fields must be properly initialized");
    }
    
    DWORD written;
    
    DWORD result = SetFilePointer(fileHandle, index * sizeof(employee), nullptr, FILE_BEGIN);
    if (result == INVALID_SET_FILE_POINTER) {
        ThrowLastError("SetFilePointer failed for write at index " + std::to_string(index));
    }
    
    if (!WriteFile(fileHandle, &emp, sizeof(employee), &written, nullptr)) {
        ThrowLastError("WriteFile failed at index " + std::to_string(index));
    }
    
    if (written != sizeof(employee)) {
        throw std::runtime_error("Incomplete write operation at index " + std::to_string(index));
    }
    
    if (!FlushFileBuffers(fileHandle)) {
        PrintError("Failed to flush file buffers after write");
    }
}

// Record count
int FileManager::getRecordCount(){
    DWORD size = GetFileSize(fileHandle, nullptr);
    if (size == INVALID_FILE_SIZE) {
        ThrowLastError("GetFileSize failed");
    }
    
    int count = size / sizeof(employee);
    if (count * sizeof(employee) != size) {
        PrintError("File size is not a multiple of employee structure size");
    }
    
    return count;
}

// Destructor
FileManager::~FileManager(){
    if (fileHandle != nullptr && fileHandle != INVALID_HANDLE_VALUE) {
        FlushFileBuffers(fileHandle);
        CloseHandle(fileHandle);
    }
}