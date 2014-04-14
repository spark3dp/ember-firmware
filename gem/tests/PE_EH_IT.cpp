/* 
 * File:   PE_EH_IT.cpp
 * Author: Richard Greene
 *
 * Created on Apr 14, 2014, 4:05:38 PM
 */

#include <stdlib.h>
#include <iostream>

/*
 * Simple C++ Test Suite
 */

void test1() {
    std::cout << "PrintEngine/EventHandler integration test 1" << std::endl;
    
    // create an event handler
    
    // connect it to the print engine
    
    // create a printer state machine
    
    // start handling events
}

void test2() {
//    std::cout << "PE_EH_IT test 2" << std::endl;
//    std::cout << "%TEST_FAILED% time=0 testname=test2 (PE_EH_IT) message=error message sample" << std::endl;
}

int main(int argc, char** argv) {
    std::cout << "%SUITE_STARTING% PE_EH_IT" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% test1 (PE_EH_IT)" << std::endl;
    test1();
    std::cout << "%TEST_FINISHED% time=0 test1 (PE_EH_IT)" << std::endl;

    std::cout << "%TEST_STARTED% test2 (PE_EH_IT)\n" << std::endl;
    test2();
    std::cout << "%TEST_FINISHED% time=0 test2 (PE_EH_IT)" << std::endl;

    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (EXIT_SUCCESS);
}

