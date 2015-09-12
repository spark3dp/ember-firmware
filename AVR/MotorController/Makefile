#  File: Makefile
#
#  This file is part of the Ember Motor Controller firmware.
#
#  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
#
#  Authors:
#  Jason Lefley
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 2 of the License, or
#  (at your option) any later version.
#
#  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
#  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
#  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
#  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.

CC	:= avr-gcc
CXX	:= avr-g++
LD	:= avr-ld
OBJCOPY	:= avr-objcopy
OBJDUMP	:= avr-objdump
SIZE	:= avr-size
MKDIR_P := mkdir -p
SMG = smg
SED_I = sed -i .BAK
MV = mv
SREC_CAT = srec_cat

TARGET = MotorController
SOURCES = $(wildcard *.c *.cpp)
SOURCES += StateMachine.cpp
DEPS = $(addsuffix .d, $(basename $(SOURCES)))
OBJECTS = $(addsuffix .o, $(basename $(SOURCES)))

# Set to 1 for debug build
DEBUG = 0

# select MCU
MCU = atmega328p

AVRDUDE_PROG := -c usbtiny
AVRDUDE_MCU = m328p -F

# Compiler flags used for C and C++
CPPFLAGS = -pipe -g -Os -mmcu=$(MCU) -Wall -fdata-sections -ffunction-sections -DF_CPU=8000000UL
CPPFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CPPFLAGS += -MMD -MP


# C compiler flags
CFLAGS = -std=gnu99 

# C++ compiler flags
CXXFLAGS = -std=c++11

# Linker flags
LDFLAGS = -Wl,-Tlinker_script_atmega328p.x,--gc-sections -lm

################################################################
# Configuration for building tests
TEST_CXX = g++
TEST_CC = gcc
TEST_CPPFLAGS = -I. -MMD -MP
TEST_CXXFLAGS = -g -O0 -std=c++11 -Wall -pedantic
TEST_CFLAGS = -g -O0 -std=gnu99 -Wall -pedantic
TEST_LDFLAGS = -lcppunit
TEST_TARGET = TestSuite
TEST_DIR = test
TEST_BUILD_DIR = test_build
TEST_SOURCES = $(wildcard $(TEST_DIR)/*.cpp) CommandBuffer.cpp Command.cpp AxisSettings.cpp EventQueue.cpp
TEST_OBJECTS = $(addprefix $(TEST_BUILD_DIR)/, $(addsuffix .o, $(basename $(TEST_SOURCES))))
TEST_DEPS = $(addsuffix .d, $(basename $(TEST_OBJECTS)))
################################################################

################################################################
# Configuration for generating state machine
SM_TARGET = StateMachine
SM_SOURCE = $(SM_TARGET).cpp
SM_HEADER = $(SM_TARGET)_smdefs.h
SM_DECL = $(SM_TARGET).sm

# v: Verbose
# E: Optimize for embedded target
# F: Output graph figure as PDF
# N: Nest switch statements for state machine handling
# l: Suppress line directives
# D: Defer tagged code event generation rather than invoking state machine recursively
SMG_OPTS = -v -F -N -l -D

# Data type of state and event codes
SM_CODE_TYPE = uint8_t
# Pass in to preprocessor so type can be referenced in source code
CPPFLAGS += -DSM_EVENT_CODE_TYPE=$(SM_CODE_TYPE)
TEST_CPPFLAGS += -DSM_EVENT_CODE_TYPE=$(SM_CODE_TYPE)
################################################################

ifeq ($(DEBUG), 1)
	CPPFLAGS += -DDEBUG
	# Non-floating point printf support
	#LDFLAGS += -Wl,-u,vfprintf -lprintf_min
	# Floating point printf support
	LDFLAGS += -Wl,-u,vfprintf -lprintf_flt
	# Enable state machine trace output and program space message string storage
	SMG_OPTS += -T -p
else
	# Disable state/event name/description lookup function generation
	SMG_OPTS += -E
endif

$(TARGET): $(TARGET).elf
	@$(SIZE) -C --mcu=$(MCU) $<

$(TARGET).elf: $(OBJECTS)
	@echo " Linking file:  $@"
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^
	@$(OBJDUMP) -h -S $@ > $(@:.elf=.lss)
	@$(OBJCOPY) -j .text -j .data -O ihex $@ $(@:.elf=.hex)
	@$(OBJCOPY) -j .text -j .data -O binary $@ $(@:.elf=.bin)

release: $(TARGET).elf
	@$(MV) $(TARGET).hex $(TARGET).org.hex
	@# Use srec_cat to add the length of the maximum address of the program data (including the
	@# maximum itself, high water + 1) as a 32 bit integer immediately after the last vector
	@# Last vector address = 0x32 (SPM READY vector address) + 0x2 = 0x34 words = 0x68 bytes
	@# This length is read by the bootloader when calculating the checksum
	@# Note that the end address (0x6a) must match the program section start hard coded in the linker script
	@$(SREC_CAT) $(TARGET).org.hex -Intel -exclude 0x68 0x6a -MAximum_Little_Endian 0x68 2 -fill 0xff -over \( $(TARGET).org.hex -Intel \) -Output $(TARGET).max.hex -Intel
	@$(SREC_CAT) $(TARGET).max.hex -Intel -o $(TARGET).hex -Intel --address_length=2 --line_length=44
	@# Print and write crc16 to file in hex format
	@$(SREC_CAT) $(TARGET).hex -Intel -CRC16_Little_Endian 0x10000 -XMODEM -crop 0x10000 0x10002 -o - -hex-dump  | cut -c 11-13 -c 14-15 | tee $(TARGET).crc16 | xargs -0 printf ' CRC16: %s'

%.o: %.c
	@echo " Building file: $<"
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $<

%.o: %.cpp
	@echo " Building file: $<"
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $<

i2c.c plan_line.c CommandMap.cpp: $(SM_HEADER)

clean:
	rm -rf $(DEPS) $(OBJECTS) $(TEST_DEPS) $(TEST_OBJECTS) $(addprefix $(TARGET), .elf .map .lss .hex .org.hex .max.hex .bin .crc16) $(TEST_TARGET)
	rm -rf $(addprefix $(SM_TARGET), .pml S.dot C.dot S.pdf C.pdf) *.BAK

install: $(TARGET).elf
	avrdude $(AVRDUDE_PROG) -V -p $(AVRDUDE_MCU) -U flash:w:$(<:.elf=.hex)

test: $(TEST_TARGET)

$(TEST_BUILD_DIR)/%.o: %.c
	@echo " Building C file $<"
	@$(MKDIR_P) $(dir $@)
	@$(TEST_CC) $(TEST_CPPFLAGS) $(TEST_CFLAGS) -o $@ -c $<

$(TEST_BUILD_DIR)/%.o: %.cpp
	@echo " Building C++ file $<"
	@$(MKDIR_P) $(dir $@)
	@$(TEST_CXX) $(TEST_CPPFLAGS) $(TEST_CXXFLAGS) -o $@ -c $<

$(TEST_TARGET): $(TEST_OBJECTS)
	@$(TEST_CXX) $(TEST_CPPFLAGS) $(TEST_CXXFLAGS) $(TEST_LDFLAGS) -o $@ $^
	@./$(TEST_TARGET)

sm: $(SM_SOURCE)

clean_sm:
	rm -rf $(SM_HEADER) $(SM_SOURCE)

$(SM_SOURCE) $(SM_HEADER): $(SM_DECL)
	@$(SMG) $(SMG_OPTS) $(SM_TARGET)
	@# Compile state machine as C++
	@mv $(SM_TARGET).c $(SM_SOURCE)
	@# Replace int types with uint8_t for efficiency on AVR
	@$(SED_I) 's/[[:space:]]int[[:space:]]/ $(SM_CODE_TYPE) /g' $(SM_SOURCE)
	@$(SED_I) 's/[[:space:]]int[[:space:]]/ $(SM_CODE_TYPE) /g' $(SM_HEADER)
	@$(SED_I) 's/(int)/($(SM_CODE_TYPE))/g' $(SM_HEADER)

.PHONY: $(TEST_TARGET)

# Include automatically generated dependencies
-include $(DEPS)
-include $(TEST_DEPS)

