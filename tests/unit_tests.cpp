#include <gtest/gtest.h>
#include <windows.h>
#include <cstring>
#include <vector>

#include "../include/employee.h"
#include "../include/file_manager.h"
#include "../include/lock_manager.h"
#include "../include/protocol.h"

//
// employee
//

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

TEST(EmployeeTest, NameBoundary)
{
    employee emp;

    strcpy(emp.name, "123456789");

    EXPECT_STREQ(emp.name, "123456789");
    EXPECT_EQ(emp.name[9], '\0');
}

//
// FileManager
//

class FileManagerTest : public ::testing::Test
{
protected:
    std::string filename;

    void SetUp() override
    {
        filename = "test_employees.dat";
        DeleteFile(filename.c_str());
    }

    void TearDown() override
    {
        DeleteFile(filename.c_str());
    }

    std::vector<employee> makeRecords(int count)
    {
        std::vector<employee> records(count);

        for (int i = 0; i < count; ++i)
        {
            records[i].num = i + 100;
            sprintf(records[i].name, "Emp%d", i);
            records[i].hours = (i + 1) * 10.0;
        }

        return records;
    }
};

TEST_F(FileManagerTest, CreateAndInitialize)
{
    auto records = makeRecords(3);

    FileManager fm(filename);
    fm.initialize(records);

    EXPECT_EQ(fm.getRecordCount(), 3);
}

TEST_F(FileManagerTest, ReadRecord)
{
    auto records = makeRecords(5);

    FileManager fm(filename);
    fm.initialize(records);

    employee emp = fm.readRecord(2);

    EXPECT_EQ(emp.num, 102);
    EXPECT_STREQ(emp.name, "Emp2");
    EXPECT_DOUBLE_EQ(emp.hours, 30.0);
}

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

TEST_F(FileManagerTest, RecordCount)
{
    auto records = makeRecords(7);

    FileManager fm(filename);
    fm.initialize(records);

    EXPECT_EQ(fm.getRecordCount(), 7);
}

//
// LockManager
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

TEST_F(LockManagerTest, ReadLockUnlock)
{
    EXPECT_TRUE(lockManager->lockRead(0));
    EXPECT_TRUE(lockManager->unlockRead(0));
}

TEST_F(LockManagerTest, WriteLockUnlock)
{
    EXPECT_TRUE(lockManager->lockWrite(0));
    EXPECT_TRUE(lockManager->unlockWrite(0));
}

TEST_F(LockManagerTest, MultipleReaders)
{
    EXPECT_TRUE(lockManager->lockRead(1));
    EXPECT_TRUE(lockManager->lockRead(1));

    EXPECT_TRUE(lockManager->unlockRead(1));
    EXPECT_TRUE(lockManager->unlockRead(1));
}

TEST_F(LockManagerTest, DifferentRecordsIndependent)
{
    EXPECT_TRUE(lockManager->lockRead(0));
    EXPECT_TRUE(lockManager->lockWrite(1));
    EXPECT_TRUE(lockManager->lockRead(2));

    EXPECT_TRUE(lockManager->unlockRead(0));
    EXPECT_TRUE(lockManager->unlockWrite(1));
    EXPECT_TRUE(lockManager->unlockRead(2));
}

TEST_F(LockManagerTest, DestructorSafe)
{
    LockManager* lm = new LockManager(3);

    EXPECT_TRUE(lm->lockRead(0));
    EXPECT_TRUE(lm->lockWrite(1));

    delete lm;
}

//
// Protocol
//

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
// Integration
//

class IntegrationTest : public ::testing::Test
{
protected:
    std::string filename;

    void SetUp() override
    {
        filename = "integration_test.dat";
        DeleteFile(filename.c_str());
    }

    void TearDown() override
    {
        DeleteFile(filename.c_str());
    }
};

TEST_F(IntegrationTest, ReadThenWriteWorkflow)
{
    std::vector<employee> data(3);

    for (int i = 0; i < 3; ++i)
    {
        data[i].num = i;
        sprintf(data[i].name, "User%d", i);
        data[i].hours = i * 10.0;
    }

    FileManager fm(filename);
    fm.initialize(data);

    LockManager lm(3);

    EXPECT_TRUE(lm.lockRead(1));

    employee emp = fm.readRecord(1);

    EXPECT_EQ(emp.num, 1);

    EXPECT_TRUE(lm.unlockRead(1));

    employee updated;
    updated.num = 999;
    strcpy(updated.name, "Updated");
    updated.hours = 888.0;

    EXPECT_TRUE(lm.lockWrite(1));

    fm.writeRecord(1, updated);

    EXPECT_TRUE(lm.unlockWrite(1));

    EXPECT_TRUE(lm.lockRead(1));

    employee result = fm.readRecord(1);

    EXPECT_EQ(result.num, 999);
    EXPECT_STREQ(result.name, "Updated");
    EXPECT_DOUBLE_EQ(result.hours, 888.0);

    EXPECT_TRUE(lm.unlockRead(1));
}

TEST_F(IntegrationTest, ParallelSimulation)
{
    std::vector<employee> data(2);

    data[0].num = 1;
    strcpy(data[0].name, "First");
    data[0].hours = 10.0;

    data[1].num = 2;
    strcpy(data[1].name, "Second");
    data[1].hours = 20.0;

    FileManager fm(filename);
    fm.initialize(data);

    LockManager lm(2);

    EXPECT_TRUE(lm.lockRead(0));
    EXPECT_TRUE(lm.lockWrite(1));

    employee updated;
    updated.num = 999;
    strcpy(updated.name, "New");
    updated.hours = 555.0;

    fm.writeRecord(1, updated);

    EXPECT_TRUE(lm.unlockRead(0));
    EXPECT_TRUE(lm.unlockWrite(1));

    employee r0 = fm.readRecord(0);
    employee r1 = fm.readRecord(1);

    EXPECT_EQ(r0.num, 1);
    EXPECT_EQ(r1.num, 999);
    EXPECT_STREQ(r1.name, "New");
}