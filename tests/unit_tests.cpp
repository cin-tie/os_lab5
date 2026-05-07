#include <gtest/gtest.h>
#include <windows.h>
#include <cstring>
#include <vector>
#include <thread>
#include <chrono>

#include "../include/employee.h"
#include "../include/file_manager.h"
#include "../include/lock_manager.h"
#include "../include/protocol.h"

//
// employee structure tests
//

// Test: Default constructor initializes all fields to zero
TEST(EmployeeTest, DefaultInitialization)
{
    employee emp;

    EXPECT_EQ(emp.num, 0);
    EXPECT_DOUBLE_EQ(emp.hours, 0.0);

    for (int i = 0; i < NAME_SIZE; ++i)
    {
        EXPECT_EQ(emp.name[i], 0);
    }
}

// Test: Setting values works correctly
TEST(EmployeeTest, SetValues)
{
    employee emp;

    emp.num = 123;
    strcpy(emp.name, "John");
    emp.hours = 40.5;

    EXPECT_EQ(emp.num, 123);
    EXPECT_STREQ(emp.name, "John");
    EXPECT_DOUBLE_EQ(emp.hours, 40.5);
}

// Test: Name field handles maximum length correctly
TEST(EmployeeTest, NameBoundary)
{
    employee emp;

    strcpy(emp.name, "123456789");

    EXPECT_STREQ(emp.name, "123456789");
    EXPECT_EQ(emp.name[9], '\0');
}

//
// FileManager tests
//

class FileManagerTest : public ::testing::Test
{
protected:
    std::string filename;

    void SetUp() override
    {
        filename = "test_employees.dat";
        DeleteFileA(filename.c_str());
    }

    void TearDown() override
    {
        DeleteFileA(filename.c_str());
    }

    std::vector<employee> makeRecords(int count)
    {
        std::vector<employee> records(count);

        for (int i = 0; i < count; ++i)
        {
            records[i].num = i + 1;  // Start from 1, not 0
            sprintf(records[i].name, "Emp%d", i);
            records[i].hours = (i + 1) * 10.0;
        }

        return records;
    }
};

// Test: File creation and initialization with records
TEST_F(FileManagerTest, CreateAndInitialize)
{
    auto records = makeRecords(3);

    FileManager fm(filename);
    fm.initialize(records);

    EXPECT_EQ(fm.getRecordCount(), 3);
}

// Test: Reading a record by index returns correct data
TEST_F(FileManagerTest, ReadRecord)
{
    auto records = makeRecords(5);

    FileManager fm(filename);
    fm.initialize(records);

    employee emp = fm.readRecord(2);

    EXPECT_EQ(emp.num, 3);  // i=2 -> num = 3
    EXPECT_STREQ(emp.name, "Emp2");
    EXPECT_DOUBLE_EQ(emp.hours, 30.0);
}

// Test: Writing a record updates the file correctly
TEST_F(FileManagerTest, WriteRecord)
{
    auto records = makeRecords(3);

    FileManager fm(filename);
    fm.initialize(records);

    employee modified;
    modified.num = 999;
    strcpy(modified.name, "Modified");
    modified.hours = 99.9;

    fm.writeRecord(1, modified);

    employee result = fm.readRecord(1);

    EXPECT_EQ(result.num, 999);
    EXPECT_STREQ(result.name, "Modified");
    EXPECT_DOUBLE_EQ(result.hours, 99.9);
}

// Test: getRecordCount returns correct number of records
TEST_F(FileManagerTest, RecordCount)
{
    auto records = makeRecords(7);

    FileManager fm(filename);
    fm.initialize(records);

    EXPECT_EQ(fm.getRecordCount(), 7);
}

//
// LockManager tests
//

class LockManagerTest : public ::testing::Test
{
protected:
    LockManager* lockManager;

    void SetUp() override
    {
        lockManager = new LockManager(5);
    }

    void TearDown() override
    {
        delete lockManager;
    }
};

// Test: Basic read lock and unlock operations
TEST_F(LockManagerTest, ReadLockUnlock)
{
    EXPECT_TRUE(lockManager->lockRead(0));
    EXPECT_TRUE(lockManager->unlockRead(0));
}

// Test: Basic write lock and unlock operations
TEST_F(LockManagerTest, WriteLockUnlock)
{
    EXPECT_TRUE(lockManager->lockWrite(0));
    EXPECT_TRUE(lockManager->unlockWrite(0));
}

// Test: Multiple readers can hold read lock simultaneously
TEST_F(LockManagerTest, MultipleReaders)
{
    EXPECT_TRUE(lockManager->lockRead(1));
    EXPECT_TRUE(lockManager->lockRead(1));

    EXPECT_TRUE(lockManager->unlockRead(1));
    EXPECT_TRUE(lockManager->unlockRead(1));
}

// Test: Different records have independent locks
TEST_F(LockManagerTest, DifferentRecordsIndependent)
{
    EXPECT_TRUE(lockManager->lockRead(0));
    EXPECT_TRUE(lockManager->lockWrite(1));
    EXPECT_TRUE(lockManager->lockRead(2));

    EXPECT_TRUE(lockManager->unlockRead(0));
    EXPECT_TRUE(lockManager->unlockWrite(1));
    EXPECT_TRUE(lockManager->unlockRead(2));
}

// Test: Writer cannot acquire lock while readers are active
TEST_F(LockManagerTest, WriterBlockedByReaders)
{
    // First reader acquires lock
    EXPECT_TRUE(lockManager->lockRead(0));
    
    // Try to acquire write lock - should timeout/fail
    EXPECT_FALSE(lockManager->lockWrite(0));
    
    // Release read lock
    EXPECT_TRUE(lockManager->unlockRead(0));
    
    // Now writer can acquire
    EXPECT_TRUE(lockManager->lockWrite(0));
    EXPECT_TRUE(lockManager->unlockWrite(0));
}

// Test: Reader cannot acquire lock while writer is active
TEST_F(LockManagerTest, ReaderBlockedByWriter)
{
    // Writer acquires lock
    EXPECT_TRUE(lockManager->lockWrite(0));
    
    // Try to acquire read lock - should fail
    EXPECT_FALSE(lockManager->lockRead(0));
    
    // Release write lock
    EXPECT_TRUE(lockManager->unlockWrite(0));
    
    // Now reader can acquire
    EXPECT_TRUE(lockManager->lockRead(0));
    EXPECT_TRUE(lockManager->unlockRead(0));
}

// Test: Destructor handles cleanup safely
TEST_F(LockManagerTest, DestructorSafe)
{
    LockManager* lm = new LockManager(3);

    EXPECT_TRUE(lm->lockRead(0));
    EXPECT_TRUE(lm->lockWrite(1));

    delete lm;
}

// Test: Invalid index returns false
TEST_F(LockManagerTest, InvalidIndex)
{
    EXPECT_FALSE(lockManager->lockRead(-1));
    EXPECT_FALSE(lockManager->lockRead(100));
    EXPECT_FALSE(lockManager->lockWrite(-1));
    EXPECT_FALSE(lockManager->lockWrite(100));
    EXPECT_FALSE(lockManager->unlockRead(-1));
    EXPECT_FALSE(lockManager->unlockWrite(-1));
}

//
// Protocol tests
//

// Test: Request structure fields are set correctly
TEST(ProtocolTest, RequestFields)
{
    Request req;

    req.type = CommandType::READ_LOCK;
    req.recordId = 42;
    req.data.num = 100;
    strcpy(req.data.name, "Test");
    req.data.hours = 50.0;

    EXPECT_EQ(req.type, CommandType::READ_LOCK);
    EXPECT_EQ(req.recordId, 42);
    EXPECT_EQ(req.data.num, 100);
    EXPECT_STREQ(req.data.name, "Test");
    EXPECT_DOUBLE_EQ(req.data.hours, 50.0);
}

// Test: Response structure fields are set correctly
TEST(ProtocolTest, ResponseFields)
{
    Response resp;

    resp.success = true;
    resp.data.num = 200;
    strcpy(resp.data.name, "Resp");
    resp.data.hours = 15.5;

    EXPECT_TRUE(resp.success);
    EXPECT_EQ(resp.data.num, 200);
    EXPECT_STREQ(resp.data.name, "Resp");
    EXPECT_DOUBLE_EQ(resp.data.hours, 15.5);
}

// Test: Request copy constructor copies all fields
TEST(ProtocolTest, RequestCopy)
{
    Request req1;

    req1.type = CommandType::WRITE_COMMIT;
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

//
// Integration tests
//

class IntegrationTest : public ::testing::Test
{
protected:
    std::string filename;

    void SetUp() override
    {
        filename = "integration_test.dat";
        DeleteFileA(filename.c_str());
    }

    void TearDown() override
    {
        DeleteFileA(filename.c_str());
    }

    std::vector<employee> makeValidRecords(int count)
    {
        std::vector<employee> records(count);
        for (int i = 0; i < count; ++i)
        {
            records[i].num = i + 1;  // Must be > 0
            sprintf(records[i].name, "User%d", i);
            records[i].hours = (i + 1) * 10.0;
        }
        return records;
    }
};

// Test: Read then write workflow with locks
TEST_F(IntegrationTest, ReadThenWriteWorkflow)
{
    auto data = makeValidRecords(3);

    FileManager fm(filename);
    fm.initialize(data);

    LockManager lm(3);

    // Read operation
    EXPECT_TRUE(lm.lockRead(1));
    employee emp = fm.readRecord(1);
    EXPECT_EQ(emp.num, 2);  // Index 1 -> num = 2
    EXPECT_TRUE(lm.unlockRead(1));

    // Write operation
    employee updated;
    updated.num = 999;
    strcpy(updated.name, "Updated");
    updated.hours = 888.0;

    EXPECT_TRUE(lm.lockWrite(1));
    fm.writeRecord(1, updated);
    EXPECT_TRUE(lm.unlockWrite(1));

    // Verify write
    EXPECT_TRUE(lm.lockRead(1));
    employee result = fm.readRecord(1);
    EXPECT_EQ(result.num, 999);
    EXPECT_STREQ(result.name, "Updated");
    EXPECT_DOUBLE_EQ(result.hours, 888.0);
    EXPECT_TRUE(lm.unlockRead(1));
}

// Test: Parallel operations on different records
TEST_F(IntegrationTest, DifferentRecordsIndependent)
{
    auto data = makeValidRecords(3);

    FileManager fm(filename);
    fm.initialize(data);

    LockManager lm(3);

    // Lock different records simultaneously
    EXPECT_TRUE(lm.lockRead(0));
    EXPECT_TRUE(lm.lockWrite(1));
    EXPECT_TRUE(lm.lockWrite(2));

    // Modify records
    employee updated1;
    updated1.num = 100;
    strcpy(updated1.name, "First");
    updated1.hours = 100.0;
    fm.writeRecord(1, updated1);

    employee updated2;
    updated2.num = 200;
    strcpy(updated2.name, "Second");
    updated2.hours = 200.0;
    fm.writeRecord(2, updated2);

    // Unlock
    EXPECT_TRUE(lm.unlockRead(0));
    EXPECT_TRUE(lm.unlockWrite(1));
    EXPECT_TRUE(lm.unlockWrite(2));

    // Verify
    employee r0 = fm.readRecord(0);
    employee r1 = fm.readRecord(1);
    employee r2 = fm.readRecord(2);

    EXPECT_EQ(r0.num, 1);
    EXPECT_EQ(r1.num, 100);
    EXPECT_EQ(r2.num, 200);
}

// Test: Writer blocked while readers active
TEST_F(IntegrationTest, WriterBlockedByReader)
{
    auto data = makeValidRecords(2);

    FileManager fm(filename);
    fm.initialize(data);

    LockManager lm(2);

    // First reader acquires lock
    EXPECT_TRUE(lm.lockRead(0));
    
    // Writer tries to acquire same record - should fail
    EXPECT_FALSE(lm.lockWrite(0));
    
    // First reader releases
    EXPECT_TRUE(lm.unlockRead(0));
    
    // Now writer can acquire
    EXPECT_TRUE(lm.lockWrite(0));
    EXPECT_TRUE(lm.unlockWrite(0));
}

// Test: Multiple readers allowed simultaneously
TEST_F(IntegrationTest, MultipleReadersAllowed)
{
    auto data = makeValidRecords(1);

    FileManager fm(filename);
    fm.initialize(data);

    LockManager lm(1);

    // Two readers can lock simultaneously
    EXPECT_TRUE(lm.lockRead(0));
    EXPECT_TRUE(lm.lockRead(0));
    
    // Verify data is readable
    employee emp = fm.readRecord(0);
    EXPECT_EQ(emp.num, 1);
    
    // Unlock both
    EXPECT_TRUE(lm.unlockRead(0));
    EXPECT_TRUE(lm.unlockRead(0));
}

// Test: Commit changes after modifications
TEST_F(IntegrationTest, CommitWorkflow)
{
    auto data = makeValidRecords(1);

    FileManager fm(filename);
    fm.initialize(data);

    LockManager lm(1);

    // Start write transaction
    EXPECT_TRUE(lm.lockWrite(0));
    
    // Read current data
    employee current = fm.readRecord(0);
    EXPECT_STREQ(current.name, "User0");
    
    // Modify locally
    employee modified = current;
    strcpy(modified.name, "Modified");
    modified.hours = 20.0;
    
    // Commit
    fm.writeRecord(0, modified);
    
    // Release lock
    EXPECT_TRUE(lm.unlockWrite(0));
    
    // Verify changes
    EXPECT_TRUE(lm.lockRead(0));
    employee result = fm.readRecord(0);
    EXPECT_STREQ(result.name, "Modified");
    EXPECT_DOUBLE_EQ(result.hours, 20.0);
    EXPECT_TRUE(lm.unlockRead(0));
}