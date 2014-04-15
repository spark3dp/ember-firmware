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
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/Event.o \
	${OBJECTDIR}/EventHandler.o \
	${OBJECTDIR}/FrontPanel.o \
	${OBJECTDIR}/I2C_Device.o \
	${OBJECTDIR}/Motor.o \
	${OBJECTDIR}/PrintEngine.o \
	${OBJECTDIR}/PrinterStateMachine.o \
	${OBJECTDIR}/main.o

# Test Directory
TESTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}/tests

# Test Files
TESTFILES= \
	${TESTDIR}/TestFiles/f1 \
	${TESTDIR}/TestFiles/f3 \
	${TESTDIR}/TestFiles/f2

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
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/gem

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/gem: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/gem ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/Event.o: Event.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -Iinclude -I/usr/include/boost -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Event.o Event.cpp

${OBJECTDIR}/EventHandler.o: EventHandler.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -Iinclude -I/usr/include/boost -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/EventHandler.o EventHandler.cpp

${OBJECTDIR}/FrontPanel.o: FrontPanel.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -Iinclude -I/usr/include/boost -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FrontPanel.o FrontPanel.cpp

${OBJECTDIR}/I2C_Device.o: I2C_Device.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -Iinclude -I/usr/include/boost -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/I2C_Device.o I2C_Device.cpp

${OBJECTDIR}/Motor.o: Motor.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -Iinclude -I/usr/include/boost -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Motor.o Motor.cpp

${OBJECTDIR}/PrintEngine.o: PrintEngine.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -Iinclude -I/usr/include/boost -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/PrintEngine.o PrintEngine.cpp

${OBJECTDIR}/PrinterStateMachine.o: PrinterStateMachine.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -Iinclude -I/usr/include/boost -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/PrinterStateMachine.o PrinterStateMachine.cpp

${OBJECTDIR}/main.o: main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -Iinclude -I/usr/include/boost -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

# Subprojects
.build-subprojects:

# Build Test Targets
.build-tests-conf: .build-conf ${TESTFILES}
${TESTDIR}/TestFiles/f1: ${TESTDIR}/tests/EventHandlerUT.o ${OBJECTFILES:%.o=%_nomain.o}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.cc}   -o ${TESTDIR}/TestFiles/f1 $^ ${LDLIBSOPTIONS} 

${TESTDIR}/TestFiles/f3: ${TESTDIR}/tests/PE_EH_IT.o ${OBJECTFILES:%.o=%_nomain.o}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.cc}   -o ${TESTDIR}/TestFiles/f3 $^ ${LDLIBSOPTIONS} 

${TESTDIR}/TestFiles/f2: ${TESTDIR}/tests/PrintEngineUT.o ${OBJECTFILES:%.o=%_nomain.o}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.cc}   -o ${TESTDIR}/TestFiles/f2 $^ ${LDLIBSOPTIONS} 


${TESTDIR}/tests/EventHandlerUT.o: tests/EventHandlerUT.cpp 
	${MKDIR} -p ${TESTDIR}/tests
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -Iinclude -I/usr/include/boost -I. -MMD -MP -MF "$@.d" -o ${TESTDIR}/tests/EventHandlerUT.o tests/EventHandlerUT.cpp


${TESTDIR}/tests/PE_EH_IT.o: tests/PE_EH_IT.cpp 
	${MKDIR} -p ${TESTDIR}/tests
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -Iinclude -I/usr/include/boost -I. -MMD -MP -MF "$@.d" -o ${TESTDIR}/tests/PE_EH_IT.o tests/PE_EH_IT.cpp


${TESTDIR}/tests/PrintEngineUT.o: tests/PrintEngineUT.cpp 
	${MKDIR} -p ${TESTDIR}/tests
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -Iinclude -I/usr/include/boost -I. -MMD -MP -MF "$@.d" -o ${TESTDIR}/tests/PrintEngineUT.o tests/PrintEngineUT.cpp


${OBJECTDIR}/Event_nomain.o: ${OBJECTDIR}/Event.o Event.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/Event.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -g -DDEBUG -Iinclude -I/usr/include/boost -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Event_nomain.o Event.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/Event.o ${OBJECTDIR}/Event_nomain.o;\
	fi

${OBJECTDIR}/EventHandler_nomain.o: ${OBJECTDIR}/EventHandler.o EventHandler.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/EventHandler.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -g -DDEBUG -Iinclude -I/usr/include/boost -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/EventHandler_nomain.o EventHandler.cpp;\
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
	    $(COMPILE.cc) -g -DDEBUG -Iinclude -I/usr/include/boost -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FrontPanel_nomain.o FrontPanel.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/FrontPanel.o ${OBJECTDIR}/FrontPanel_nomain.o;\
	fi

${OBJECTDIR}/I2C_Device_nomain.o: ${OBJECTDIR}/I2C_Device.o I2C_Device.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/I2C_Device.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -g -DDEBUG -Iinclude -I/usr/include/boost -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/I2C_Device_nomain.o I2C_Device.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/I2C_Device.o ${OBJECTDIR}/I2C_Device_nomain.o;\
	fi

${OBJECTDIR}/Motor_nomain.o: ${OBJECTDIR}/Motor.o Motor.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/Motor.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -g -DDEBUG -Iinclude -I/usr/include/boost -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Motor_nomain.o Motor.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/Motor.o ${OBJECTDIR}/Motor_nomain.o;\
	fi

${OBJECTDIR}/PrintEngine_nomain.o: ${OBJECTDIR}/PrintEngine.o PrintEngine.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/PrintEngine.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -g -DDEBUG -Iinclude -I/usr/include/boost -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/PrintEngine_nomain.o PrintEngine.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/PrintEngine.o ${OBJECTDIR}/PrintEngine_nomain.o;\
	fi

${OBJECTDIR}/PrinterStateMachine_nomain.o: ${OBJECTDIR}/PrinterStateMachine.o PrinterStateMachine.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/PrinterStateMachine.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -g -DDEBUG -Iinclude -I/usr/include/boost -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/PrinterStateMachine_nomain.o PrinterStateMachine.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/PrinterStateMachine.o ${OBJECTDIR}/PrinterStateMachine_nomain.o;\
	fi

${OBJECTDIR}/main_nomain.o: ${OBJECTDIR}/main.o main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	@NMOUTPUT=`${NM} ${OBJECTDIR}/main.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -g -DDEBUG -Iinclude -I/usr/include/boost -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main_nomain.o main.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/main.o ${OBJECTDIR}/main_nomain.o;\
	fi

# Run Test Targets
.test-conf:
	@if [ "${TEST}" = "" ]; \
	then  \
	    ${TESTDIR}/TestFiles/f1 || true; \
	    ${TESTDIR}/TestFiles/f3 || true; \
	    ${TESTDIR}/TestFiles/f2 || true; \
	else  \
	    ./${TEST} || true; \
	fi

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/gem

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
