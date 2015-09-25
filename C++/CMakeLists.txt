cmake_minimum_required(VERSION 2.8)
project(smith)

include_directories(
    include
    /usr/include/ImageMagick
)

add_executable(smith
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
    main.cpp
    utils.cpp
)

target_link_libraries(smith
    pthread
    zpp
    iw
    udev
    Magick++
    tar
    SDL
    SDL_image
)

install(TARGETS smith RUNTIME DESTINATION /usr/local/bin)
