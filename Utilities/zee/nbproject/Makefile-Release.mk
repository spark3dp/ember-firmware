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
	${OBJECTDIR}/_ext/1445226307/I2C_Device.o \
	${OBJECTDIR}/_ext/1445226307/Logger.o \
	${OBJECTDIR}/_ext/1445226307/Motor.o \
	${OBJECTDIR}/_ext/1445226307/MotorCommand.o \
	${OBJECTDIR}/_ext/1445226307/PrinterStatus.o \
	${OBJECTDIR}/_ext/1445226307/Settings.o \
	${OBJECTDIR}/_ext/1445226307/SparkStatus.o \
	${OBJECTDIR}/_ext/1445226307/utils.o \
	${OBJECTDIR}/main.o


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
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/zee

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/zee: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/zee ${OBJECTFILES} ${LDLIBSOPTIONS} -lrt -lSDL -lSDL_image -ltar -lz -liw

${OBJECTDIR}/_ext/1445226307/I2C_Device.o: ../../C++/I2C_Device.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1445226307
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I../../C++/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1445226307/I2C_Device.o ../../C++/I2C_Device.cpp

${OBJECTDIR}/_ext/1445226307/Logger.o: ../../C++/Logger.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1445226307
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I../../C++/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1445226307/Logger.o ../../C++/Logger.cpp

${OBJECTDIR}/_ext/1445226307/Motor.o: ../../C++/Motor.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1445226307
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I../../C++/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1445226307/Motor.o ../../C++/Motor.cpp

${OBJECTDIR}/_ext/1445226307/MotorCommand.o: ../../C++/MotorCommand.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1445226307
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I../../C++/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1445226307/MotorCommand.o ../../C++/MotorCommand.cpp

${OBJECTDIR}/_ext/1445226307/PrinterStatus.o: ../../C++/PrinterStatus.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1445226307
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I../../C++/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1445226307/PrinterStatus.o ../../C++/PrinterStatus.cpp

${OBJECTDIR}/_ext/1445226307/Settings.o: ../../C++/Settings.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1445226307
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I../../C++/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1445226307/Settings.o ../../C++/Settings.cpp

${OBJECTDIR}/_ext/1445226307/SparkStatus.o: ../../C++/SparkStatus.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1445226307
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I../../C++/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1445226307/SparkStatus.o ../../C++/SparkStatus.cpp

${OBJECTDIR}/_ext/1445226307/utils.o: ../../C++/utils.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1445226307
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I../../C++/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1445226307/utils.o ../../C++/utils.cpp

${OBJECTDIR}/main.o: main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I../../C++/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/zee

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
