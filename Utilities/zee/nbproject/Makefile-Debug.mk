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
CND_PLATFORM=GNU-Linux
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
	${OBJECTDIR}/_ext/56246743/I2C_Device.o \
	${OBJECTDIR}/_ext/56246743/Logger.o \
	${OBJECTDIR}/_ext/56246743/Motor.o \
	${OBJECTDIR}/_ext/56246743/MotorCommand.o \
	${OBJECTDIR}/_ext/56246743/PrinterStatus.o \
	${OBJECTDIR}/_ext/56246743/Settings.o \
	${OBJECTDIR}/_ext/56246743/SparkStatus.o \
	${OBJECTDIR}/_ext/56246743/utils.o \
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
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/zee ${OBJECTFILES} ${LDLIBSOPTIONS} -lrt -ltar -lz -liw

${OBJECTDIR}/_ext/56246743/I2C_Device.o: ../../C++/I2C_Device.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/56246743
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I../../C++/include -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/56246743/I2C_Device.o ../../C++/I2C_Device.cpp

${OBJECTDIR}/_ext/56246743/Logger.o: ../../C++/Logger.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/56246743
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I../../C++/include -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/56246743/Logger.o ../../C++/Logger.cpp

${OBJECTDIR}/_ext/56246743/Motor.o: ../../C++/Motor.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/56246743
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I../../C++/include -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/56246743/Motor.o ../../C++/Motor.cpp

${OBJECTDIR}/_ext/56246743/MotorCommand.o: ../../C++/MotorCommand.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/56246743
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I../../C++/include -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/56246743/MotorCommand.o ../../C++/MotorCommand.cpp

${OBJECTDIR}/_ext/56246743/PrinterStatus.o: ../../C++/PrinterStatus.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/56246743
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I../../C++/include -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/56246743/PrinterStatus.o ../../C++/PrinterStatus.cpp

${OBJECTDIR}/_ext/56246743/Settings.o: ../../C++/Settings.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/56246743
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I../../C++/include -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/56246743/Settings.o ../../C++/Settings.cpp

${OBJECTDIR}/_ext/56246743/SparkStatus.o: ../../C++/SparkStatus.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/56246743
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I../../C++/include -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/56246743/SparkStatus.o ../../C++/SparkStatus.cpp

${OBJECTDIR}/_ext/56246743/utils.o: ../../C++/utils.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/56246743
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I../../C++/include -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/56246743/utils.o ../../C++/utils.cpp

${OBJECTDIR}/main.o: main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I../../C++/include -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

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
