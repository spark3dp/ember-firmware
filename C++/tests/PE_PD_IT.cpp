/* 
 * File:   PE_PD_IT.cpp
 * Author: Jason Lefley
 * PrintEngine PrintData integration test
 *
 * Created on Jul 7, 2014, 4:49:06 PM
 */

#include <stdlib.h>
#include <iostream>

void test1() {
    std::cout << "PE_PD_IT test 1" << std::endl;
}

void test2() {
    std::cout << "PE_PD_IT test 2" << std::endl;
    std::cout << "%TEST_FAILED% time=0 testname=test2 (PE_PD_IT) message=error message sample" << std::endl;
}

int main(int argc, char** argv) {
    std::cout << "%SUITE_STARTING% PE_PD_IT" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% test1 (PE_PD_IT)" << std::endl;
    test1();
    std::cout << "%TEST_FINISHED% time=0 test1 (PE_PD_IT)" << std::endl;

    std::cout << "%TEST_STARTED% test2 (PE_PD_IT)\n" << std::endl;
    test2();
    std::cout << "%TEST_FINISHED% time=0 test2 (PE_PD_IT)" << std::endl;

    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (EXIT_SUCCESS);
}

