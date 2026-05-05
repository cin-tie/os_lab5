#pragma once

#include <cstring>

const int NAME_SIZE = 10;


// Employee struct from task
struct employee
{
    int num;
    char name[NAME_SIZE];
    double hours;

    // Initialization
    employee(){
        num = 0;
        memset(name, 0, NAME_SIZE);
        hours = 0;
    }
};