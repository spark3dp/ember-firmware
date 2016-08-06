cmake_minimum_required(VERSION 2.8)
project(smith)

# Suppress warning about having target named test
cmake_policy(SET CMP0037 OLD)

# Include the NetBeans test support file
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/nb_tests.cmake)

# Find libraries and include paths
find_package(ImageMagick COMPONENTS Magick++ REQUIRED)
find_package(ZLIB REQUIRED)

# Enable C++11
# These lines must appear before any calls to add_library or add_executable
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Specify include directories here
# Also add paths to NetBeans Code Assistance in project properties
include_directories(
    include
    tests
    ${ImageMagick_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIRS}
    /usr/include/libdrm
)

# Specify hardware independent source files here
add_library(Core STATIC
    CommandInterpreter.cpp
    CommandPipe.cpp
    EventHandler.cpp
    FrontPanel.cpp
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
)

# Specify library dependencies here
set(LIBRARIES
    pthread
    zpp
    iw
    udev
    tar
    ${ImageMagick_LIBRARIES}
    ${ZLIB_LIBRARIES}
    drm
)

# Specify hardware dependent source files here
add_library(Hardware STATIC EXCLUDE_FROM_ALL
    DRM_Connector.cpp 
    DRM_Device.cpp
    DRM_DumbBuffer.cpp
    DRM_Encoder.cpp
    DRM_FrameBuffer.cpp
    DRM_Resources.cpp
    FrameBuffer.cpp 
    GPIO_Interrupt.cpp
    HardwareFactory.cpp
    I2C_Device.cpp
    GPIO.cpp
)

# Specify mock hardware source file here
add_library(MockHardware STATIC EXCLUDE_FROM_ALL
    mock_hardware/NamedPipeResource.cpp
    mock_hardware/NamedPipeI2C_Device.cpp
    mock_hardware/HardwareFactory.cpp
    mock_hardware/ImageWritingFrameBuffer.cpp
)

add_executable(smith
    main.cpp
)

# Specify compilation flags here
set(CMAKE_CXX_FLAGS_DEBUG    "-g3 -gdwarf-2")
set(CMAKE_CXX_FLAGS_RELEASE  "-O3")
set(CMAKE_CXX_FLAGS_COVERAGE "${CMAKE_CXX_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")

# Specify compile time definitions here
# DIRECTORY scope applies definitions to all sources under the directory that
# contains this file
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS
    # Define DEBUG for debug and coverage builds
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Coverage>:DEBUG>

    # ImageMagick configuration
    MAGICKCORE_HDRI_ENABLE=0
    MAGICKCORE_QUANTUM_DEPTH=16
)

set(USE_MOCK_HARDWARE FALSE CACHE BOOL "Enable to build with mock hardware")

if (${USE_MOCK_HARDWARE})
    message(STATUS "Configured to build with mock hardware")
    list(INSERT LIBRARIES 0 MockHardware)
else()
    message(STATUS "Configured to build with actual hardware")
    list(INSERT LIBRARIES 0 Hardware)
endif()

list(INSERT LIBRARIES 0 Core)

target_link_libraries(smith ${LIBRARIES})
install(TARGETS smith RUNTIME DESTINATION /usr/local/bin)

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
