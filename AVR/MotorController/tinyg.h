#ifndef tinyg_h
#define tinyg_h

#include <stdint.h>
#include <stddef.h>

#define true (1 == 1)
#define false (1 == 0)

#define AXES   1
#define MOTORS 1
#define MAGICNUM 0x12EF     // used for memory integrity assertions
#define MOTOR_1 0

typedef uint8_t stat_t;

// OS, communications and low-level status (must align with XIO_xxxx codes in xio.h)
#define  STAT_OK 0           // function completed OK
#define  STAT_ERROR 1          // generic error return (EPERM)
#define  STAT_EAGAIN 2         // function would block here (call again)
#define  STAT_NOOP 3           // function had no-operation
#define  STAT_COMPLETE 4         // operation is complete
#define STAT_TERMINATE 5       // operation terminated (gracefully)
#define STAT_RESET 6         // operation was hard reset (sig kill)
#define  STAT_EOL 7            // function returned end-of-line
#define  STAT_EOF 8            // function returned end-of-file 
#define  STAT_FILE_NOT_OPEN 9
#define  STAT_FILE_SIZE_EXCEEDED 10
#define  STAT_NO_SUCH_DEVICE 11
#define  STAT_BUFFER_EMPTY 12
#define  STAT_BUFFER_FULL 13
#define  STAT_BUFFER_FULL_FATAL 14
#define  STAT_INITIALIZING 15      // initializing - not ready for use
#define  STAT_ENTERING_BOOT_LOADER 16  // this code actually emitted from boot loader, not TinyG
#define  STAT_ERROR_17 17
#define  STAT_ERROR_18 18
#define  STAT_ERROR_19 19        // NOTE: XIO codes align to here

// Internal errors and startup messages
#define  STAT_INTERNAL_ERROR 20      // unrecoverable internal error
#define  STAT_INTERNAL_RANGE_ERROR 21  // number range other than by user input
#define  STAT_FLOATING_POINT_ERROR 22  // number conversion error
#define  STAT_DIVIDE_BY_ZERO 23
#define  STAT_INVALID_ADDRESS 24
#define  STAT_READ_ONLY_ADDRESS 25
#define  STAT_INIT_FAIL 26
#define  STAT_ALARMED 27
#define  STAT_MEMORY_FAULT 28
#define  STAT_ERROR_29 29
#define  STAT_ERROR_30 30
#define  STAT_ERROR_31 31
#define  STAT_ERROR_32 32
#define  STAT_ERROR_33 33
#define  STAT_ERROR_34 34
#define  STAT_ERROR_35 35
#define  STAT_ERROR_36 36
#define  STAT_ERROR_37 37
#define  STAT_ERROR_38 38
#define  STAT_ERROR_39 39

// Input errors (400's, if you will)
#define  STAT_UNRECOGNIZED_COMMAND 40    // parser didn't recognize the command
#define  STAT_EXPECTED_COMMAND_LETTER 41   // malformed line to parser
#define  STAT_BAD_NUMBER_FORMAT 42     // number format error
#define  STAT_INPUT_EXCEEDS_MAX_LENGTH 43  // input string is too long 
#define  STAT_INPUT_VALUE_TOO_SMALL 44   // input error: value is under minimum
#define  STAT_INPUT_VALUE_TOO_LARGE 45   // input error: value is over maximum
#define  STAT_INPUT_VALUE_RANGE_ERROR 46   // input error: value is out-of-range
#define  STAT_INPUT_VALUE_UNSUPPORTED 47   // input error: value is not supported
#define  STAT_JSON_SYNTAX_ERROR 48     // JSON input string is not well formed
#define  STAT_JSON_TOO_MANY_PAIRS 49     // JSON input string has too many JSON pairs
#define  STAT_JSON_TOO_LONG 50       // JSON output exceeds buffer size
#define  STAT_NO_BUFFER_SPACE 51       // Buffer pool is full and cannot perform this operation
#define  STAT_CONFIG_NOT_TAKEN 52      // configuration value not taken while in machining cycle
#define  STAT_ERROR_53 53
#define  STAT_ERROR_54 54
#define  STAT_ERROR_55 55
#define  STAT_ERROR_56 56
#define  STAT_ERROR_57 57
#define  STAT_ERROR_58 58
#define  STAT_ERROR_59 59

// Gcode and machining errors
#define  STAT_MINIMUM_LENGTH_MOVE_ERROR 60 // move is less than minimum length
#define  STAT_MINIMUM_TIME_MOVE_ERROR 61   // move is less than minimum time
#define  STAT_GCODE_BLOCK_SKIPPED 62     // block is too short - was skipped
#define  STAT_GCODE_INPUT_ERROR 63     // general error for gcode input 
#define  STAT_GCODE_FEEDRATE_ERROR 64    // move has no feedrate
#define  STAT_GCODE_AXIS_WORD_MISSING 65   // command requires at least one axis present
#define  STAT_MODAL_GROUP_VIOLATION 66   // gcode modal group error
#define  STAT_HOMING_CYCLE_FAILED 67     // homing cycle did not complete
#define  STAT_MAX_TRAVEL_EXCEEDED 68
#define  STAT_MAX_SPINDLE_SPEED_EXCEEDED 69
#define  STAT_ARC_SPECIFICATION_ERROR 70   // arc specification error

// Configuration options
#define NOM_SEGMENT_USEC    ((float)5000)   // nominal segment time
#define MOTOR_POLARITY 1                    //0=normal, 1=reversed

// LED debug
// May need to add #include <avr/io.h> to use these
#define PB6_OFF() PORTB &= ~(1<<PB6)
#define PB7_OFF() PORTB &= ~(1<<PB7)
#define PB6_ON() PORTB |= (1<<PB6)
#define PB7_ON() PORTB |= (1<<PB7)
#define LED_INIT() DDRB |= (1<<DDB6) | (1<<DDB7)

#endif
