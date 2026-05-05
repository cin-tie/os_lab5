#include <gtest/gtest.h>
#include <windows.h>
#include <fstream>
#include <cstring>

#include "../include/employee.h"
#include "../include/file_manager.h"
#include "../include/lock_manager.h"
#include "../include/protocol.h"

// ============= TESTS FOR employee STRUCT =============

TEST(EmployeeTest, DefaultInitialization) {
    employee emp;
    
    EXPECT_EQ(emp.num, 0);
    EXPECT_EQ(emp.hours, 0.0);
    // Name should be zero-initialized
    for(int i = 0; i < NAME_SIZE; i++) {
        EXPECT_EQ(emp.name[i], 0);
    }
}

TEST(EmployeeTest, SetValues) {
    employee emp;
    emp.num = 123;
    strcpy(emp.name, "John");
    emp.hours = 40.5;
    
    EXPECT_EQ(emp.num, 123);
    EXPECT_STREQ(emp.name, "John");
    EXPECT_DOUBLE_EQ(emp.hours, 40.5);
}

TEST(EmployeeTest, NameBoundary) {
    employee emp;
    // Test with maximum size name
    strcpy(emp.name, "123456789");
    EXPECT_STREQ(emp.name, "123456789");
    
    // Name should be null-terminated within bounds
    emp.name[NAME_SIZE - 1] = '\0';
    EXPECT_EQ(emp.name[NAME_SIZE - 1], '\0');
}

// ============= TESTS FOR FileManager =============

class FileManagerTest : public ::testing::Test {
protected:
    std::string testFilename;
    
    void SetUp() override {
        testFilename = "test_employees.dat";
        // Remove test file if exists
        DeleteFile(testFilename.c_str());
    }
    
    void TearDown() override {
        // Clean up test file
        DeleteFile(testFilename.c_str());
    }
    
    std::vector<employee> createSampleRecords(int count) {
        std::vector<employee> records(count);
        for(int i = 0; i < count; i++) {
            records[i].num = i + 100;
            sprintf(records[i].name, "Emp%d", i);
            records[i].hours = (i + 1) * 10.0;
        }
        return records;
    }
};

TEST_F(FileManagerTest, CreateFileAndInitialize) {
    std::vector<employee> records = createSampleRecords(3);
    
    FileManager fm(testFilename);
    fm.initialize(records);
    
    EXPECT_EQ(fm.getRecordCount(), 3);
}

TEST_F(FileManagerTest, ReadRecord) {
    std::vector<employee> records = createSampleRecords(5);
    
    FileManager fm(testFilename);
    fm.initialize(records);
    
    employee emp = fm.readRecord(2);
    
    EXPECT_EQ(emp.num, 102);
    EXPECT_STREQ(emp.name, "Emp2");
    EXPECT_DOUBLE_EQ(emp.hours, 30.0);
}

TEST_F(FileManagerTest, WriteRecord) {
    std::vector<employee> records = createSampleRecords(3);
    
    FileManager fm(testFilename);
    fm.initialize(records);
    
    // Modify record at index 1
    employee modified;
    modified.num = 999;
    strcpy(modified.name, "Modified");
    modified.hours = 99.9;
    
    fm.writeRecord(1, modified);
    
    employee read = fm.readRecord(1);
    EXPECT_EQ(read.num, 999);
    EXPECT_STREQ(read.name, "Modified");
    EXPECT_DOUBLE_EQ(read.hours, 99.9);
    
    // Verify other records unchanged
    employee unchanged = fm.readRecord(0);
    EXPECT_EQ(unchanged.num, 100);
}

TEST_F(FileManagerTest, GetRecordCount) {
    std::vector<employee> records = createSampleRecords(7);
    
    FileManager fm(testFilename);
    fm.initialize(records);
    
    EXPECT_EQ(fm.getRecordCount(), 7);
}

TEST_F(FileManagerTest, OverwriteFile) {
    std::vector<employee> records1 = createSampleRecords(2);
    std::vector<employee> records2 = createSampleRecords(4);
    
    FileManager fm(testFilename);
    fm.initialize(records1);
    EXPECT_EQ(fm.getRecordCount(), 2);
    
    fm.initialize(records2);
    EXPECT_EQ(fm.getRecordCount(), 4);
    
    employee emp = fm.readRecord(3);
    EXPECT_EQ(emp.num, 103);
}

// ============= TESTS FOR LockManager =============

class LockManagerTest : public ::testing::Test {
protected:
    LockManager* lockManager;
    
    void SetUp() override {
        lockManager = new LockManager(5);
    }
    
    void TearDown() override {
        delete lockManager;
    }
};

TEST_F(LockManagerTest, LockReadUnlockRead) {
    // Should not throw
    EXPECT_NO_THROW(lockManager->lockRead(0));
    EXPECT_NO_THROW(lockManager->unlockRead(0));
}

TEST_F(LockManagerTest, LockWriteUnlockWrite) {
    EXPECT_NO_THROW(lockManager->lockWrite(0));
    EXPECT_NO_THROW(lockManager->unlockWrite(0));
}

TEST_F(LockManagerTest, MultipleReadLocksSameRecord) {
    // Multiple readers should be able to lock same record
    EXPECT_NO_THROW(lockManager->lockRead(1));
    EXPECT_NO_THROW(lockManager->lockRead(1));
    EXPECT_NO_THROW(lockManager->unlockRead(1));
    EXPECT_NO_THROW(lockManager->unlockRead(1));
}

TEST_F(LockManagerTest, DifferentRecords) {
    // Should be able to lock different records independently
    EXPECT_NO_THROW(lockManager->lockRead(0));
    EXPECT_NO_THROW(lockManager->lockWrite(1));
    EXPECT_NO_THROW(lockManager->lockRead(2));
    
    EXPECT_NO_THROW(lockManager->unlockRead(0));
    EXPECT_NO_THROW(lockManager->unlockWrite(1));
    EXPECT_NO_THROW(lockManager->unlockRead(2));
}

TEST_F(LockManagerTest, LockManagerDestructor) {
    // Test that destructor doesn't crash
    LockManager* lm = new LockManager(3);
    lm->lockRead(0);
    lm->lockWrite(1);
    delete lm;  // Should clean up mutexes
}

// ============= TESTS FOR Request/Response Protocol =============

TEST(ProtocolTest, RequestSize) {
    Request req;
    req.type = CommandType::READ;
    req.recordId = 42;
    req.data.num = 100;
    strcpy(req.data.name, "Test");
    req.data.hours = 50.0;
    
    EXPECT_EQ(sizeof(Request), sizeof(CommandType) + sizeof(int) + sizeof(employee));
}

TEST(ProtocolTest, ResponseSize) {
    Response resp;
    resp.success = true;
    resp.data.num = 200;
    
    // Just check that size is reasonable (not zero and not too large)
    EXPECT_GT(sizeof(Response), 0);
    EXPECT_LT(sizeof(Response), 100);
    
    // Check that it's at least the size of its members
    EXPECT_GE(sizeof(Response), sizeof(bool) + sizeof(employee) - 4); // Allow for padding
}

TEST(ProtocolTest, RequestCopy) {
    Request req1;
    req1.type = CommandType::WRITE;
    req1.recordId = 5;
    req1.data.num = 123;
    strcpy(req1.data.name, "Copy");
    req1.data.hours = 75.5;
    
    Request req2 = req1;
    
    EXPECT_EQ(req2.type, req1.type);
    EXPECT_EQ(req2.recordId, req1.recordId);
    EXPECT_EQ(req2.data.num, req1.data.num);
    EXPECT_STREQ(req2.data.name, req1.data.name);
    EXPECT_DOUBLE_EQ(req2.data.hours, req1.data.hours);
}

// ============= INTEGRATION TESTS =============

class IntegrationTest : public ::testing::Test {
protected:
    std::string testFilename;
    
    void SetUp() override {
        testFilename = "integration_test.dat";
        DeleteFile(testFilename.c_str());
    }
    
    void TearDown() override {
        DeleteFile(testFilename.c_str());
    }
};

TEST_F(IntegrationTest, FileManagerWithLockManagerWorkflow) {
    // Create test data
    std::vector<employee> initialData(3);
    for(int i = 0; i < 3; i++) {
        initialData[i].num = i;
        sprintf(initialData[i].name, "User%d", i);
        initialData[i].hours = i * 10.0;
    }
    
    // Initialize file
    FileManager fm(testFilename);
    fm.initialize(initialData);
    EXPECT_EQ(fm.getRecordCount(), 3);
    
    // Create lock manager
    LockManager lm(3);
    
    // Perform read with lock
    lm.lockRead(1);
    employee emp = fm.readRecord(1);
    EXPECT_EQ(emp.num, 1);
    lm.unlockRead(1);
    
    // Perform write with lock
    employee newEmp;
    newEmp.num = 999;
    strcpy(newEmp.name, "Updated");
    newEmp.hours = 888.0;
    
    lm.lockWrite(1);
    fm.writeRecord(1, newEmp);
    lm.unlockWrite(1);
    
    // Verify write
    lm.lockRead(1);
    emp = fm.readRecord(1);
    EXPECT_EQ(emp.num, 999);
    EXPECT_STREQ(emp.name, "Updated");
    EXPECT_DOUBLE_EQ(emp.hours, 888.0);
    lm.unlockRead(1);
}

TEST_F(IntegrationTest, ConcurrentReadWriteSimulation) {
    std::vector<employee> initialData(2);
    initialData[0].num = 1;
    strcpy(initialData[0].name, "First");
    initialData[0].hours = 10.0;
    initialData[1].num = 2;
    strcpy(initialData[1].name, "Second");
    initialData[1].hours = 20.0;
    
    FileManager fm(testFilename);
    fm.initialize(initialData);
    
    LockManager lm(2);
    
    // Simulate multiple operations on different records
    lm.lockRead(0);
    lm.lockWrite(1);
    
    employee e0 = fm.readRecord(0);
    employee e1;
    e1.num = 999;
    strcpy(e1.name, "New");
    e1.hours = 555.0;
    fm.writeRecord(1, e1);
    
    lm.unlockRead(0);
    lm.unlockWrite(1);
    
    // Verify results
    employee read0 = fm.readRecord(0);
    employee read1 = fm.readRecord(1);
    
    EXPECT_EQ(read0.num, 1);
    EXPECT_EQ(read1.num, 999);
    EXPECT_STREQ(read1.name, "New");
}
