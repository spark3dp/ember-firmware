#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_DLIB_EXT=so
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/CommandInterpreter.o \
	${OBJECTDIR}/CommandPipe.o \
	${OBJECTDIR}/EventHandler.o \
	${OBJECTDIR}/FrontPanel.o \
	${OBJECTDIR}/GPIO_Interrupt.o \
	${OBJECTDIR}/I2C_Device.o \
	${OBJECTDIR}/I2C_Resource.o \
	${OBJECTDIR}/LayerSettings.o \
	${OBJECTDIR}/Logger.o \
	${OBJECTDIR}/Motor.o \
	${OBJECTDIR}/MotorCommand.o \
	${OBJECTDIR}/NetworkInterface.o \
	${OBJECTDIR}/PrintData.o \
	${OBJECTDIR}/PrintDataDirectory.o \
	${OBJECTDIR}/PrintDataZip.o \
	${OBJECTDIR}/PrintEngine.o \
	${OBJECTDIR}/PrintFileStorage.o \
	${OBJECTDIR}/PrinterStateMachine.o \
	${OBJECTDIR}/PrinterStatus.o \
	${OBJECTDIR}/PrinterStatusQueue.o \
	${OBJECTDIR}/Projector.o \
	${OBJECTDIR}/Screen.o \
	${OBJECTDIR}/ScreenBuilder.o \
	${OBJECTDIR}/Settings.o \
	${OBJECTDIR}/Signals.o \
	${OBJECTDIR}/SparkStatus.o \
	${OBJECTDIR}/StandardIn.o \
	${OBJECTDIR}/TarGzFile.o \
	${OBJECTDIR}/TerminalUI.o \
	${OBJECTDIR}/Thermometer.o \
	${OBJECTDIR}/Timer.o \
	${OBJECTDIR}/UdevMonitor.o \
	${OBJECTDIR}/main.o \
	${OBJECTDIR}/utils.o

# Test Directory
TESTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}/tests

# Test Files
TESTFILES= \
	${TESTDIR}/TestFiles/f5 \
	${TESTDIR}/TestFiles/f1 \
	${TESTDIR}/TestFiles/f10 \
	${TESTDIR}/TestFiles/f11 \
	${TESTDIR}/TestFiles/f4 \
	${TESTDIR}/TestFiles/f12 \
	${TESTDIR}/TestFiles/f7 \
	${TESTDIR}/TestFiles/f13 \
	${TESTDIR}/TestFiles/f8 \
	${TESTDIR}/TestFiles/f2 \
	${TESTDIR}/TestFiles/f9 \
	${TESTDIR}/TestFiles/f6

# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/smith

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/smith: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/smith ${OBJECTFILES} ${LDLIBSOPTIONS} -lrt -lSDL -lSDL_image -ltar -lz -liw -lzpp -lMagick++ -ludev

${OBJECTDIR}/CommandInterpreter.o: CommandInterpreter.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/CommandInterpreter.o CommandInterpreter.cpp

${OBJECTDIR}/CommandPipe.o: CommandPipe.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/CommandPipe.o CommandPipe.cpp

${OBJECTDIR}/EventHandler.o: EventHandler.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/EventHandler.o EventHandler.cpp

${OBJECTDIR}/FrontPanel.o: FrontPanel.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FrontPanel.o FrontPanel.cpp

${OBJECTDIR}/GPIO_Interrupt.o: GPIO_Interrupt.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/GPIO_Interrupt.o GPIO_Interrupt.cpp

${OBJECTDIR}/I2C_Device.o: I2C_Device.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/I2C_Device.o I2C_Device.cpp

${OBJECTDIR}/I2C_Resource.o: I2C_Resource.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/I2C_Resource.o I2C_Resource.cpp

${OBJECTDIR}/LayerSettings.o: LayerSettings.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/LayerSettings.o LayerSettings.cpp

${OBJECTDIR}/Logger.o: Logger.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Logger.o Logger.cpp

${OBJECTDIR}/Motor.o: Motor.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Motor.o Motor.cpp

${OBJECTDIR}/MotorCommand.o: MotorCommand.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/MotorCommand.o MotorCommand.cpp

${OBJECTDIR}/NetworkInterface.o: NetworkInterface.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/NetworkInterface.o NetworkInterface.cpp

${OBJECTDIR}/PrintData.o: PrintData.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/PrintData.o PrintData.cpp

${OBJECTDIR}/PrintDataDirectory.o: PrintDataDirectory.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/PrintDataDirectory.o PrintDataDirectory.cpp

${OBJECTDIR}/PrintDataZip.o: PrintDataZip.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/PrintDataZip.o PrintDataZip.cpp

${OBJECTDIR}/PrintEngine.o: PrintEngine.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/PrintEngine.o PrintEngine.cpp

${OBJECTDIR}/PrintFileStorage.o: PrintFileStorage.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/PrintFileStorage.o PrintFileStorage.cpp

${OBJECTDIR}/PrinterStateMachine.o: PrinterStateMachine.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/PrinterStateMachine.o PrinterStateMachine.cpp

${OBJECTDIR}/PrinterStatus.o: PrinterStatus.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/PrinterStatus.o PrinterStatus.cpp

${OBJECTDIR}/PrinterStatusQueue.o: PrinterStatusQueue.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/PrinterStatusQueue.o PrinterStatusQueue.cpp

${OBJECTDIR}/Projector.o: Projector.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Projector.o Projector.cpp

${OBJECTDIR}/Screen.o: Screen.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Screen.o Screen.cpp

${OBJECTDIR}/ScreenBuilder.o: ScreenBuilder.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/ScreenBuilder.o ScreenBuilder.cpp

${OBJECTDIR}/Settings.o: Settings.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Settings.o Settings.cpp

${OBJECTDIR}/Signals.o: Signals.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Signals.o Signals.cpp

${OBJECTDIR}/SparkStatus.o: SparkStatus.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/SparkStatus.o SparkStatus.cpp

${OBJECTDIR}/StandardIn.o: StandardIn.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/StandardIn.o StandardIn.cpp

${OBJECTDIR}/TarGzFile.o: TarGzFile.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/TarGzFile.o TarGzFile.cpp

${OBJECTDIR}/TerminalUI.o: TerminalUI.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/TerminalUI.o TerminalUI.cpp

${OBJECTDIR}/Thermometer.o: Thermometer.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Thermometer.o Thermometer.cpp

${OBJECTDIR}/Timer.o: Timer.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Timer.o Timer.cpp

${OBJECTDIR}/UdevMonitor.o: UdevMonitor.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/UdevMonitor.o UdevMonitor.cpp

${OBJECTDIR}/main.o: main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

${OBJECTDIR}/utils.o: utils.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/utils.o utils.cpp

# Subprojects
.build-subprojects:

# Build Test Targets
.build-tests-conf: .build-conf ${TESTFILES}
${TESTDIR}/TestFiles/f5: -lrt

${TESTDIR}/TestFiles/f5: -lSDL_image

${TESTDIR}/TestFiles/f5: -lSDL

${TESTDIR}/TestFiles/f5: -ltar

${TESTDIR}/TestFiles/f5: -lz

${TESTDIR}/TestFiles/f5: -liw

${TESTDIR}/TestFiles/f5: -lzpp

${TESTDIR}/TestFiles/f5: -lMagick++

${TESTDIR}/TestFiles/f5: -ludev

${TESTDIR}/TestFiles/f5: ${TESTDIR}/tests/CommandInterpreterUT.o ${OBJECTFILES:%.o=%_nomain.o}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.cc}   -o ${TESTDIR}/TestFiles/f5 $^ ${LDLIBSOPTIONS} 

${TESTDIR}/TestFiles/f1: -lrt

${TESTDIR}/TestFiles/f1: -lSDL_image

${TESTDIR}/TestFiles/f1: -lSDL

${TESTDIR}/TestFiles/f1: -ltar

${TESTDIR}/TestFiles/f1: -lz

${TESTDIR}/TestFiles/f1: -liw

${TESTDIR}/TestFiles/f1: -lzpp

${TESTDIR}/TestFiles/f1: -lMagick++

${TESTDIR}/TestFiles/f1: -ludev

${TESTDIR}/TestFiles/f1: ${TESTDIR}/tests/EventHandlerUT.o ${OBJECTFILES:%.o=%_nomain.o}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.cc}  -lrt -o ${TESTDIR}/TestFiles/f1 $^ ${LDLIBSOPTIONS} 

${TESTDIR}/TestFiles/f10: -lrt

${TESTDIR}/TestFiles/f10: -lSDL_image

${TESTDIR}/TestFiles/f10: -lSDL

${TESTDIR}/TestFiles/f10: -ltar

${TESTDIR}/TestFiles/f10: -lz

${TESTDIR}/TestFiles/f10: -liw

${TESTDIR}/TestFiles/f10: -lzpp

${TESTDIR}/TestFiles/f10: -lMagick++

${TESTDIR}/TestFiles/f10: -ludev

${TESTDIR}/TestFiles/f10: ${TESTDIR}/tests/FrontPanelTest.o ${OBJECTFILES:%.o=%_nomain.o}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.cc}   -o ${TESTDIR}/TestFiles/f10 $^ ${LDLIBSOPTIONS} 

${TESTDIR}/TestFiles/f11: -lrt

${TESTDIR}/TestFiles/f11: -lSDL_image

${TESTDIR}/TestFiles/f11: -lSDL

${TESTDIR}/TestFiles/f11: -ltar

${TESTDIR}/TestFiles/f11: -lz

${TESTDIR}/TestFiles/f11: -liw

${TESTDIR}/TestFiles/f11: -lzpp

${TESTDIR}/TestFiles/f11: -lMagick++

${TESTDIR}/TestFiles/f11: -ludev

${TESTDIR}/TestFiles/f11: ${TESTDIR}/tests/LayerSettingsUT.o ${OBJECTFILES:%.o=%_nomain.o}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.cc}   -o ${TESTDIR}/TestFiles/f11 $^ ${LDLIBSOPTIONS} 

${TESTDIR}/TestFiles/f4: -lrt

${TESTDIR}/TestFiles/f4: -lSDL_image

${TESTDIR}/TestFiles/f4: -lSDL

${TESTDIR}/TestFiles/f4: -ltar

${TESTDIR}/TestFiles/f4: -lz

${TESTDIR}/TestFiles/f4: -liw

${TESTDIR}/TestFiles/f4: -lzpp

${TESTDIR}/TestFiles/f4: -lMagick++

${TESTDIR}/TestFiles/f4: -ludev

${TESTDIR}/TestFiles/f4: ${TESTDIR}/tests/NetworkIFUT.o ${OBJECTFILES:%.o=%_nomain.o}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.cc}   -o ${TESTDIR}/TestFiles/f4 $^ ${LDLIBSOPTIONS} 

${TESTDIR}/TestFiles/f12: -lrt

${TESTDIR}/TestFiles/f12: -lSDL_image

${TESTDIR}/TestFiles/f12: -lSDL

${TESTDIR}/TestFiles/f12: -ltar

${TESTDIR}/TestFiles/f12: -lz

${TESTDIR}/TestFiles/f12: -liw

${TESTDIR}/TestFiles/f12: -lzpp

${TESTDIR}/TestFiles/f12: -lMagick++

${TESTDIR}/TestFiles/f12: -ludev

${TESTDIR}/TestFiles/f12: ${TESTDIR}/tests/PrintDataDirectoryUT.o ${OBJECTFILES:%.o=%_nomain.o}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.cc}   -o ${TESTDIR}/TestFiles/f12 $^ ${LDLIBSOPTIONS} 

${TESTDIR}/TestFiles/f7: -lrt

${TESTDIR}/TestFiles/f7: -lSDL_image

${TESTDIR}/TestFiles/f7: -lSDL

${TESTDIR}/TestFiles/f7: -ltar

${TESTDIR}/TestFiles/f7: -lz

${TESTDIR}/TestFiles/f7: -liw

${TESTDIR}/TestFiles/f7: -lzpp

${TESTDIR}/TestFiles/f7: -lMagick++

${TESTDIR}/TestFiles/f7: -ludev

${TESTDIR}/TestFiles/f7: ${TESTDIR}/tests/PrintDataUT.o ${OBJECTFILES:%.o=%_nomain.o}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.cc}   -o ${TESTDIR}/TestFiles/f7 $^ ${LDLIBSOPTIONS} 

${TESTDIR}/TestFiles/f13: -lrt

${TESTDIR}/TestFiles/f13: -lSDL_image

${TESTDIR}/TestFiles/f13: -lSDL

${TESTDIR}/TestFiles/f13: -ltar

${TESTDIR}/TestFiles/f13: -lz

${TESTDIR}/TestFiles/f13: -liw

${TESTDIR}/TestFiles/f13: -lzpp

${TESTDIR}/TestFiles/f13: -lMagick++

${TESTDIR}/TestFiles/f13: -ludev

${TESTDIR}/TestFiles/f13: ${TESTDIR}/tests/PrintDataZipUT.o ${OBJECTFILES:%.o=%_nomain.o}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.cc}   -o ${TESTDIR}/TestFiles/f13 $^ ${LDLIBSOPTIONS} 

${TESTDIR}/TestFiles/f8: -lrt

${TESTDIR}/TestFiles/f8: -lSDL_image

${TESTDIR}/TestFiles/f8: -lSDL

${TESTDIR}/TestFiles/f8: -ltar

${TESTDIR}/TestFiles/f8: -lz

${TESTDIR}/TestFiles/f8: -liw

${TESTDIR}/TestFiles/f8: -lzpp

${TESTDIR}/TestFiles/f8: -lMagick++

${TESTDIR}/TestFiles/f8: -ludev

${TESTDIR}/TestFiles/f8: ${TESTDIR}/tests/PE_PD_IT.o ${OBJECTFILES:%.o=%_nomain.o}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.cc}   -o ${TESTDIR}/TestFiles/f8 $^ ${LDLIBSOPTIONS} 

${TESTDIR}/TestFiles/f2: -lrt

${TESTDIR}/TestFiles/f2: -lSDL_image

${TESTDIR}/TestFiles/f2: -lSDL

${TESTDIR}/TestFiles/f2: -ltar

${TESTDIR}/TestFiles/f2: -lz

${TESTDIR}/TestFiles/f2: -liw

${TESTDIR}/TestFiles/f2: -lzpp

${TESTDIR}/TestFiles/f2: -lMagick++

${TESTDIR}/TestFiles/f2: -ludev

${TESTDIR}/TestFiles/f2: ${TESTDIR}/tests/PrintEngineUT.o ${OBJECTFILES:%.o=%_nomain.o}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.cc}  -lrt -o ${TESTDIR}/TestFiles/f2 $^ ${LDLIBSOPTIONS} 

${TESTDIR}/TestFiles/f9: -lrt

${TESTDIR}/TestFiles/f9: -lSDL_image

${TESTDIR}/TestFiles/f9: -lSDL

${TESTDIR}/TestFiles/f9: -ltar

${TESTDIR}/TestFiles/f9: -lz

${TESTDIR}/TestFiles/f9: -liw

${TESTDIR}/TestFiles/f9: -lzpp

${TESTDIR}/TestFiles/f9: -lMagick++

${TESTDIR}/TestFiles/f9: -ludev

${TESTDIR}/TestFiles/f9: ${TESTDIR}/tests/ScreenUT.o ${OBJECTFILES:%.o=%_nomain.o}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.cc}   -o ${TESTDIR}/TestFiles/f9 $^ ${LDLIBSOPTIONS} 

${TESTDIR}/TestFiles/f6: -lrt

${TESTDIR}/TestFiles/f6: -lSDL_image

${TESTDIR}/TestFiles/f6: -lSDL

${TESTDIR}/TestFiles/f6: -ltar

${TESTDIR}/TestFiles/f6: -lz

${TESTDIR}/TestFiles/f6: -liw

${TESTDIR}/TestFiles/f6: -lzpp

${TESTDIR}/TestFiles/f6: -lMagick++

${TESTDIR}/TestFiles/f6: -ludev

${TESTDIR}/TestFiles/f6: ${TESTDIR}/tests/SettingsUT.o ${OBJECTFILES:%.o=%_nomain.o}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.cc}   -o ${TESTDIR}/TestFiles/f6 $^ ${LDLIBSOPTIONS} 


${TESTDIR}/tests/CommandInterpreterUT.o: tests/CommandInterpreterUT.cpp 
	${MKDIR} -p ${TESTDIR}/tests
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -I. -include tests/support/FileUtils.hpp -MMD -MP -MF "$@.d" -o ${TESTDIR}/tests/CommandInterpreterUT.o tests/CommandInterpreterUT.cpp


${TESTDIR}/tests/EventHandlerUT.o: tests/EventHandlerUT.cpp 
	${MKDIR} -p ${TESTDIR}/tests
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -I. -include tests/support/FileUtils.hpp -MMD -MP -MF "$@.d" -o ${TESTDIR}/tests/EventHandlerUT.o tests/EventHandlerUT.cpp


${TESTDIR}/tests/FrontPanelTest.o: tests/FrontPanelTest.cpp 
	${MKDIR} -p ${TESTDIR}/tests
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -I. -include tests/support/FileUtils.hpp -MMD -MP -MF "$@.d" -o ${TESTDIR}/tests/FrontPanelTest.o tests/FrontPanelTest.cpp


${TESTDIR}/tests/LayerSettingsUT.o: tests/LayerSettingsUT.cpp 
	${MKDIR} -p ${TESTDIR}/tests
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -I. -include tests/support/FileUtils.hpp -MMD -MP -MF "$@.d" -o ${TESTDIR}/tests/LayerSettingsUT.o tests/LayerSettingsUT.cpp


${TESTDIR}/tests/NetworkIFUT.o: tests/NetworkIFUT.cpp 
	${MKDIR} -p ${TESTDIR}/tests
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -I. -include tests/support/FileUtils.hpp -MMD -MP -MF "$@.d" -o ${TESTDIR}/tests/NetworkIFUT.o tests/NetworkIFUT.cpp


${TESTDIR}/tests/PrintDataDirectoryUT.o: tests/PrintDataDirectoryUT.cpp 
	${MKDIR} -p ${TESTDIR}/tests
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -I. -include tests/support/FileUtils.hpp -MMD -MP -MF "$@.d" -o ${TESTDIR}/tests/PrintDataDirectoryUT.o tests/PrintDataDirectoryUT.cpp


${TESTDIR}/tests/PrintDataUT.o: tests/PrintDataUT.cpp 
	${MKDIR} -p ${TESTDIR}/tests
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -I. -include tests/support/FileUtils.hpp -MMD -MP -MF "$@.d" -o ${TESTDIR}/tests/PrintDataUT.o tests/PrintDataUT.cpp


${TESTDIR}/tests/PrintDataZipUT.o: tests/PrintDataZipUT.cpp 
	${MKDIR} -p ${TESTDIR}/tests
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -I. -I. -include tests/support/FileUtils.hpp -MMD -MP -MF "$@.d" -o ${TESTDIR}/tests/PrintDataZipUT.o tests/PrintDataZipUT.cpp


${TESTDIR}/tests/PE_PD_IT.o: tests/PE_PD_IT.cpp 
	${MKDIR} -p ${TESTDIR}/tests
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -I. -include tests/support/FileUtils.hpp -MMD -MP -MF "$@.d" -o ${TESTDIR}/tests/PE_PD_IT.o tests/PE_PD_IT.cpp


${TESTDIR}/tests/PrintEngineUT.o: tests/PrintEngineUT.cpp 
	${MKDIR} -p ${TESTDIR}/tests
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -I. -include tests/support/FileUtils.hpp -MMD -MP -MF "$@.d" -o ${TESTDIR}/tests/PrintEngineUT.o tests/PrintEngineUT.cpp


${TESTDIR}/tests/ScreenUT.o: tests/ScreenUT.cpp 
	${MKDIR} -p ${TESTDIR}/tests
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -I. -include tests/support/FileUtils.hpp -MMD -MP -MF "$@.d" -o ${TESTDIR}/tests/ScreenUT.o tests/ScreenUT.cpp


${TESTDIR}/tests/SettingsUT.o: tests/SettingsUT.cpp 
	${MKDIR} -p ${TESTDIR}/tests
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -I. -include tests/support/FileUtils.hpp -MMD -MP -MF "$@.d" -o ${TESTDIR}/tests/SettingsUT.o tests/SettingsUT.cpp


${OBJECTDIR}/CommandInterpreter_nomain.o: ${OBJECTDIR}/CommandInterpreter.o CommandInterpreter.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/CommandInterpreter.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/CommandInterpreter_nomain.o CommandInterpreter.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/CommandInterpreter.o ${OBJECTDIR}/CommandInterpreter_nomain.o;\
	fi

${OBJECTDIR}/CommandPipe_nomain.o: ${OBJECTDIR}/CommandPipe.o CommandPipe.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/CommandPipe.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/CommandPipe_nomain.o CommandPipe.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/CommandPipe.o ${OBJECTDIR}/CommandPipe_nomain.o;\
	fi

${OBJECTDIR}/EventHandler_nomain.o: ${OBJECTDIR}/EventHandler.o EventHandler.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/EventHandler.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/EventHandler_nomain.o EventHandler.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/EventHandler.o ${OBJECTDIR}/EventHandler_nomain.o;\
	fi

${OBJECTDIR}/FrontPanel_nomain.o: ${OBJECTDIR}/FrontPanel.o FrontPanel.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/FrontPanel.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FrontPanel_nomain.o FrontPanel.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/FrontPanel.o ${OBJECTDIR}/FrontPanel_nomain.o;\
	fi

${OBJECTDIR}/GPIO_Interrupt_nomain.o: ${OBJECTDIR}/GPIO_Interrupt.o GPIO_Interrupt.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/GPIO_Interrupt.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/GPIO_Interrupt_nomain.o GPIO_Interrupt.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/GPIO_Interrupt.o ${OBJECTDIR}/GPIO_Interrupt_nomain.o;\
	fi

${OBJECTDIR}/I2C_Device_nomain.o: ${OBJECTDIR}/I2C_Device.o I2C_Device.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/I2C_Device.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/I2C_Device_nomain.o I2C_Device.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/I2C_Device.o ${OBJECTDIR}/I2C_Device_nomain.o;\
	fi

${OBJECTDIR}/I2C_Resource_nomain.o: ${OBJECTDIR}/I2C_Resource.o I2C_Resource.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/I2C_Resource.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/I2C_Resource_nomain.o I2C_Resource.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/I2C_Resource.o ${OBJECTDIR}/I2C_Resource_nomain.o;\
	fi

${OBJECTDIR}/LayerSettings_nomain.o: ${OBJECTDIR}/LayerSettings.o LayerSettings.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/LayerSettings.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/LayerSettings_nomain.o LayerSettings.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/LayerSettings.o ${OBJECTDIR}/LayerSettings_nomain.o;\
	fi

${OBJECTDIR}/Logger_nomain.o: ${OBJECTDIR}/Logger.o Logger.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/Logger.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Logger_nomain.o Logger.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/Logger.o ${OBJECTDIR}/Logger_nomain.o;\
	fi

${OBJECTDIR}/Motor_nomain.o: ${OBJECTDIR}/Motor.o Motor.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/Motor.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Motor_nomain.o Motor.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/Motor.o ${OBJECTDIR}/Motor_nomain.o;\
	fi

${OBJECTDIR}/MotorCommand_nomain.o: ${OBJECTDIR}/MotorCommand.o MotorCommand.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/MotorCommand.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/MotorCommand_nomain.o MotorCommand.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/MotorCommand.o ${OBJECTDIR}/MotorCommand_nomain.o;\
	fi

${OBJECTDIR}/NetworkInterface_nomain.o: ${OBJECTDIR}/NetworkInterface.o NetworkInterface.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/NetworkInterface.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/NetworkInterface_nomain.o NetworkInterface.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/NetworkInterface.o ${OBJECTDIR}/NetworkInterface_nomain.o;\
	fi

${OBJECTDIR}/PrintData_nomain.o: ${OBJECTDIR}/PrintData.o PrintData.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/PrintData.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/PrintData_nomain.o PrintData.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/PrintData.o ${OBJECTDIR}/PrintData_nomain.o;\
	fi

${OBJECTDIR}/PrintDataDirectory_nomain.o: ${OBJECTDIR}/PrintDataDirectory.o PrintDataDirectory.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/PrintDataDirectory.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/PrintDataDirectory_nomain.o PrintDataDirectory.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/PrintDataDirectory.o ${OBJECTDIR}/PrintDataDirectory_nomain.o;\
	fi

${OBJECTDIR}/PrintDataZip_nomain.o: ${OBJECTDIR}/PrintDataZip.o PrintDataZip.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/PrintDataZip.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/PrintDataZip_nomain.o PrintDataZip.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/PrintDataZip.o ${OBJECTDIR}/PrintDataZip_nomain.o;\
	fi

${OBJECTDIR}/PrintEngine_nomain.o: ${OBJECTDIR}/PrintEngine.o PrintEngine.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/PrintEngine.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/PrintEngine_nomain.o PrintEngine.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/PrintEngine.o ${OBJECTDIR}/PrintEngine_nomain.o;\
	fi

${OBJECTDIR}/PrintFileStorage_nomain.o: ${OBJECTDIR}/PrintFileStorage.o PrintFileStorage.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/PrintFileStorage.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/PrintFileStorage_nomain.o PrintFileStorage.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/PrintFileStorage.o ${OBJECTDIR}/PrintFileStorage_nomain.o;\
	fi

${OBJECTDIR}/PrinterStateMachine_nomain.o: ${OBJECTDIR}/PrinterStateMachine.o PrinterStateMachine.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/PrinterStateMachine.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/PrinterStateMachine_nomain.o PrinterStateMachine.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/PrinterStateMachine.o ${OBJECTDIR}/PrinterStateMachine_nomain.o;\
	fi

${OBJECTDIR}/PrinterStatus_nomain.o: ${OBJECTDIR}/PrinterStatus.o PrinterStatus.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/PrinterStatus.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/PrinterStatus_nomain.o PrinterStatus.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/PrinterStatus.o ${OBJECTDIR}/PrinterStatus_nomain.o;\
	fi

${OBJECTDIR}/PrinterStatusQueue_nomain.o: ${OBJECTDIR}/PrinterStatusQueue.o PrinterStatusQueue.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/PrinterStatusQueue.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/PrinterStatusQueue_nomain.o PrinterStatusQueue.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/PrinterStatusQueue.o ${OBJECTDIR}/PrinterStatusQueue_nomain.o;\
	fi

${OBJECTDIR}/Projector_nomain.o: ${OBJECTDIR}/Projector.o Projector.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/Projector.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Projector_nomain.o Projector.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/Projector.o ${OBJECTDIR}/Projector_nomain.o;\
	fi

${OBJECTDIR}/Screen_nomain.o: ${OBJECTDIR}/Screen.o Screen.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/Screen.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Screen_nomain.o Screen.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/Screen.o ${OBJECTDIR}/Screen_nomain.o;\
	fi

${OBJECTDIR}/ScreenBuilder_nomain.o: ${OBJECTDIR}/ScreenBuilder.o ScreenBuilder.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/ScreenBuilder.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/ScreenBuilder_nomain.o ScreenBuilder.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/ScreenBuilder.o ${OBJECTDIR}/ScreenBuilder_nomain.o;\
	fi

${OBJECTDIR}/Settings_nomain.o: ${OBJECTDIR}/Settings.o Settings.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/Settings.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Settings_nomain.o Settings.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/Settings.o ${OBJECTDIR}/Settings_nomain.o;\
	fi

${OBJECTDIR}/Signals_nomain.o: ${OBJECTDIR}/Signals.o Signals.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/Signals.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Signals_nomain.o Signals.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/Signals.o ${OBJECTDIR}/Signals_nomain.o;\
	fi

${OBJECTDIR}/SparkStatus_nomain.o: ${OBJECTDIR}/SparkStatus.o SparkStatus.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/SparkStatus.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/SparkStatus_nomain.o SparkStatus.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/SparkStatus.o ${OBJECTDIR}/SparkStatus_nomain.o;\
	fi

${OBJECTDIR}/StandardIn_nomain.o: ${OBJECTDIR}/StandardIn.o StandardIn.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/StandardIn.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/StandardIn_nomain.o StandardIn.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/StandardIn.o ${OBJECTDIR}/StandardIn_nomain.o;\
	fi

${OBJECTDIR}/TarGzFile_nomain.o: ${OBJECTDIR}/TarGzFile.o TarGzFile.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/TarGzFile.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/TarGzFile_nomain.o TarGzFile.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/TarGzFile.o ${OBJECTDIR}/TarGzFile_nomain.o;\
	fi

${OBJECTDIR}/TerminalUI_nomain.o: ${OBJECTDIR}/TerminalUI.o TerminalUI.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/TerminalUI.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/TerminalUI_nomain.o TerminalUI.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/TerminalUI.o ${OBJECTDIR}/TerminalUI_nomain.o;\
	fi

${OBJECTDIR}/Thermometer_nomain.o: ${OBJECTDIR}/Thermometer.o Thermometer.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/Thermometer.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Thermometer_nomain.o Thermometer.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/Thermometer.o ${OBJECTDIR}/Thermometer_nomain.o;\
	fi

${OBJECTDIR}/Timer_nomain.o: ${OBJECTDIR}/Timer.o Timer.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/Timer.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Timer_nomain.o Timer.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/Timer.o ${OBJECTDIR}/Timer_nomain.o;\
	fi

${OBJECTDIR}/UdevMonitor_nomain.o: ${OBJECTDIR}/UdevMonitor.o UdevMonitor.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/UdevMonitor.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/UdevMonitor_nomain.o UdevMonitor.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/UdevMonitor.o ${OBJECTDIR}/UdevMonitor_nomain.o;\
	fi

${OBJECTDIR}/main_nomain.o: ${OBJECTDIR}/main.o main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/main.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main_nomain.o main.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/main.o ${OBJECTDIR}/main_nomain.o;\
	fi

${OBJECTDIR}/utils_nomain.o: ${OBJECTDIR}/utils.o utils.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/utils.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -Iinclude -I/usr/include/ImageMagick -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/utils_nomain.o utils.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/utils.o ${OBJECTDIR}/utils_nomain.o;\
	fi

# Run Test Targets
.test-conf:
	@if [ "${TEST}" = "" ]; \
	then  \
	    ${TESTDIR}/TestFiles/f5 || true; \
	    ${TESTDIR}/TestFiles/f1 || true; \
	    ${TESTDIR}/TestFiles/f10 || true; \
	    ${TESTDIR}/TestFiles/f11 || true; \
	    ${TESTDIR}/TestFiles/f4 || true; \
	    ${TESTDIR}/TestFiles/f12 || true; \
	    ${TESTDIR}/TestFiles/f7 || true; \
	    ${TESTDIR}/TestFiles/f13 || true; \
	    ${TESTDIR}/TestFiles/f8 || true; \
	    ${TESTDIR}/TestFiles/f2 || true; \
	    ${TESTDIR}/TestFiles/f9 || true; \
	    ${TESTDIR}/TestFiles/f6 || true; \
	else  \
	    ./${TEST} || true; \
	fi

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/smith

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
