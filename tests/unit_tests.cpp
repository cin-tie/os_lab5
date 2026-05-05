#include <gtest/gtest.h>
#include "../include/employee.h"

TEST(EmployeeTest, InitValues) {
    employee emp;

    EXPECT_EQ(emp.num, 0);
    EXPECT_EQ(emp.hours, 0);
}

TEST(EmployeeTest, NameCopy) {
    employee emp;

    strcpy(emp.name, "Ivan");

    EXPECT_STREQ(
        emp.name,
        "Ivan"
    );
}