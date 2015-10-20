cmake_minimum_required(VERSION 2.8)
project(smith)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/nb_tests.cmake)

# Specify include directories here
# Also add paths to NetBeans Code Assistance in project properties
include_directories(
    include
    tests
    /usr/include/ImageMagick
)

# Specify hardware independent source files here
add_library(Core STATIC
    CommandInterpreter.cpp
    CommandPipe.cpp
    EventHandler.cpp
    FrontPanel.cpp
    GPIO_Interrupt.cpp
    I2C_Resource.cpp
    ImageProcessor.cpp
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
   
    I2C_Device.cpp
)

# Specify library dependencies here
set(LIBRARIES
    Core 
    pthread
    zpp
    iw
    udev
    Magick++
    tar
    SDL
    SDL_image
)

# Specify hardware dependent source files here
add_library(Hardware STATIC EXCLUDE_FROM_ALL
    FrameBuffer.cpp 
    HardwareFactory.cpp
)

# Specify mock hardware source file here
add_library(MockHardware STATIC EXCLUDE_FROM_ALL
    mock_hardware/NamedPipeResource.cpp
    mock_hardware/NamedPipeI2C_Device.cpp
    mock_hardware/HardwareFactory.cpp
)

# Specify automated tests here
# The first argument (f1, etc.) must correspond to the value of the Output
# property specified in the test's folder/linker properties so NetBeans can
# launch the correct test executable when debugging
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
add_nb_test(f13 tests/ImageProcessorUT.cpp)

# Specify compilation flags here.
set(CMAKE_CXX_FLAGS          "-std=c++0x")
set(CMAKE_CXX_FLAGS_DEBUG    "-g3 -gdwarf-2")
set(CMAKE_CXX_FLAGS_RELEASE  "-O3")
set(CMAKE_CXX_FLAGS_COVERAGE "${CMAKE_CXX_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")

if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "Coverage")
    add_definitions(-DDEBUG)
endif()

set(USE_MOCK_HARDWARE FALSE CACHE BOOL "Enable to build with mock hardware")

add_executable(smith main.cpp)

if (${USE_MOCK_HARDWARE})
    message(STATUS "Configured to build with mock hardware")
    list(APPEND LIBRARIES MockHardware)
else()
    message(STATUS "Configured to build with actual hardware")
    list(APPEND LIBRARIES Hardware)
endif()

target_link_libraries(smith ${LIBRARIES})
install(TARGETS smith RUNTIME DESTINATION /usr/local/bin)
