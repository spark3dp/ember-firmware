CC	:= avr-gcc
CXX	:= avr-g++
LD	:= avr-ld
OBJCOPY	:= avr-objcopy
OBJDUMP	:= avr-objdump
SIZE	:= avr-size
MKDIR_P := mkdir -p
DOT = dot
SMG = smg
SED_I = sed -i .BAK

TARGET = MotorController
SOURCES = $(wildcard *.c *.cpp)
SOURCES += StateMachine.cpp
DEPS = $(addsuffix .d, $(basename $(SOURCES)))
OBJECTS = $(addsuffix .o, $(basename $(SOURCES)))

# Set to 1 for debug build
DEBUG = 1

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
LDFLAGS = -Wl,--gc-sections -lm

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
SM_DIAGRAM = $(SM_TARGET).pdf
SM_GRAPHVIZ = $(SM_TARGET)S.dot

# v: Verbose
# E: Optimize for embedded target
# F: Output graph figure as PDF
# N: Nest switch statements for state machine handling
# l: Suppress line directives
# D: Defer tagged code event generation rather than invoking state machine recursively
SMG_OPTS = -v -E -F -N -l -D

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
	# Enable state machine trace output
	SMG_OPTS += -T
else
	# Enable linker relaxation
	LDFLAGS += -Wl,--relax
endif

$(TARGET): $(TARGET).elf
	@$(SIZE) -C --mcu=$(MCU) $<

$(TARGET).elf: $(OBJECTS)
	@echo " Linking file:  $@"
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^
	@$(OBJDUMP) -h -S $@ > $(@:.elf=.lss)
	@$(OBJCOPY) -j .text -j .data -O ihex $@ $(@:.elf=.hex)
	@$(OBJCOPY) -j .text -j .data -O binary $@ $(@:.elf=.bin)

%.o: %.c
	@echo " Building file: $<"
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $<

%.o: %.cpp
	@echo " Building file: $<"
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $<

plan_line.c CommandMap.cpp: $(SM_HEADER)

clean:
	rm -rf $(DEPS) $(OBJECTS) $(TEST_DEPS) $(TEST_OBJECTS) $(addprefix $(TARGET), .elf .map .lss .hex .bin) $(TEST_TARGET)
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

$(SM_SOURCE) $(SM_HEADER) $(SM_GRAPHVIZ): $(SM_DECL)
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

