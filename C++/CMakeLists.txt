cmake_minimum_required(VERSION 2.8)
project(smith)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/nb_tests.cmake)

include_directories(
    include
    /usr/include/ImageMagick
    tests
)

 set(CMAKE_CXX_FLAGS_DEBUG "-g3 -gdwarf-2")
 set(CMAKE_CXX_FLAGS_RELEASE "-O3")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG)
endif()

add_library(core STATIC
    CommandInterpreter.cpp
    CommandPipe.cpp
    EventHandler.cpp
    FrontPanel.cpp
    GPIO_Interrupt.cpp
    I2C_Device.cpp
    I2C_Resource.cpp
    LayerSettings.cpp
    Logger.cpp
    Motor.cpp
    MotorCommand.cpp
    NetworkInterface.cpp
    PrintData.cpp
    PrintDataDirectory.cpp
    PrintDataZip.cpp
    PrintEngine.cpp
    PrintFileStorage.cpp
    PrinterStateMachine.cpp
    PrinterStatus.cpp
    PrinterStatusQueue.cpp
    Projector.cpp
    Screen.cpp
    ScreenBuilder.cpp
    Settings.cpp
    Signals.cpp
    SparkStatus.cpp
    StandardIn.cpp
    TarGzFile.cpp
    TerminalUI.cpp
    Thermometer.cpp
    Timer.cpp
    UdevMonitor.cpp
    utils.cpp
)

set(LIBRARIES
    core 
    pthread
    zpp
    iw
    udev
    Magick++
    tar
    SDL
    SDL_image
)

add_executable(smith main.cpp)
target_link_libraries(smith ${LIBRARIES})

install(TARGETS smith RUNTIME DESTINATION /usr/local/bin)

add_nb_test(f1 tests/CommandInterpreterUT.cpp)
add_nb_test(f2 tests/EventHandlerUT.cpp)
add_nb_test(f3 tests/FrontPanelTest.cpp)
add_nb_test(f4 tests/LayerSettingsUT.cpp)
add_nb_test(f5 tests/NetworkIFUT.cpp)
add_nb_test(f6 tests/PrintDataDirectoryUT.cpp)
add_nb_test(f7 tests/PrintDataUT.cpp)
add_nb_test(f8 tests/PrintDataZipUT.cpp)
add_nb_test(f9 tests/PE_PD_IT.cpp)
add_nb_test(f10 tests/PrintEngineUT.cpp)
add_nb_test(f11 tests/ScreenUT.cpp)
add_nb_test(f12 tests/SettingsUT.cpp)
