/* 
 * File:   PrintEngineUT.cpp
 * Author: greener
 *
 * Created on Apr 8, 2014, 4:13:10 PM
 */

#include <stdlib.h>
#include <iostream>
#include <PrintEngine.h>
#include <stdio.h>

/*
 * Simple C++ Test Suite
 */

void test1() {
    std::cout << "PrintEngineUT test 1" << std::endl;
    
    printf("\tabout to instantiate printer\n");
    PrinterStateMachine psm;
    printf("\tabout to initiate printer\n");
    psm.initiate();
    
    printf("\tabout to process sleep event\n");
    psm.process_event(EvSleep());
    
    printf("\tabout to process wake event\n");    
    psm.process_event(EvWake());
     
    printf("\tabout to process reset event\n");
    psm.process_event(EvReset());
    
    printf("\tabout to process door opened event\n");
    psm.process_event(EvDoorOpened());
    
    printf("\tabout to process door closed event\n");    
    psm.process_event(EvDoorClosed());
    
    printf("\tabout to process door opened event again\n");
    psm.process_event(EvDoorOpened());

    printf("\tabout to process reset event again\n");
    psm.process_event(EvReset());
    
    printf("\tabout to shut down\n");
}

void test2() {
//    std::cout << "PrintEngineUT test 2" << std::endl;
//    std::cout << "%TEST_FAILED% time=0 testname=test2 (PrintEngineUT) message=error message sample" << std::endl;
}

int main(int argc, char** argv) {
    std::cout << "%SUITE_STARTING% PrintEngineUT" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% test1 (PrintEngineUT)" << std::endl;
    test1();
    std::cout << "%TEST_FINISHED% time=0 test1 (PrintEngineUT)" << std::endl;

    std::cout << "%TEST_STARTED% test2 (PrintEngineUT)\n" << std::endl;
    test2();
    std::cout << "%TEST_FINISHED% time=0 test2 (PrintEngineUT)" << std::endl;

    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (EXIT_SUCCESS);
}

