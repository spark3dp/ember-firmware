#MODEL=uno
MODEL=pro328
#PORT=/dev/tty.usbmodem1411
PORT=/dev/tty.usbserial-AH00ZSKO
BUILD_DIR=.build/${MODEL}
TARGET=firmware

all: build upload serial

build: src/*
	ino build -m ${MODEL} --ldflags=-Tlinker_script_atmega328p.x,--gc-sections
	mv ${BUILD_DIR}/${TARGET}.hex ${BUILD_DIR}/${TARGET}.org.hex
	@# Use srec_cat to add the length of the maximum address of the program data (including the
	@# maximum itself, high water + 1) as a 32 bit integer immediately after the last vector
	@# Last vector address = 0x32 (SPM READY vector address) + 0x2 = 0x34 words = 0x68 bytes
	@# This length is read by the bootloader when calculating the checksum
	@# Note that the end address (0x6a) must match the program section start hard coded in the linker script
	srec_cat ${BUILD_DIR}/${TARGET}.org.hex -Intel -exclude 0x68 0x6a -MAximum_Little_Endian 0x68 2 -fill 0xff -over \( ${BUILD_DIR}/${TARGET}.org.hex -Intel \) -Output ${BUILD_DIR}/${TARGET}.max.hex -Intel
	srec_cat ${BUILD_DIR}/${TARGET}.max.hex -Intel -o ${BUILD_DIR}/${TARGET}.hex -Intel --address_length=2 --line_length=44
	@# Print and write crc16 to file in hex format
	srec_cat ${BUILD_DIR}/${TARGET}.hex -Intel -CRC16_Little_Endian 0x10000 -XMODEM -crop 0x10000 0x10002 -o - -hex-dump  | cut -c 11-13 -c 14-15 | tee ${BUILD_DIR}/${TARGET}.crc16

upload:
	ino upload -p ${PORT}

serial:
	ino serial 

serialp:
	ino serial -p ${PORT}

docs:
	doxygen doxyconf

prog: build
	@rm -f firmware.hex
	@ln -s .build/${MODEL}/firmware.hex .
	avrdude -pm328p -cusbtiny -V -Uflash:w:firmware.hex:i

clean:
	ino clean

.PHONY: docs upload serial clean

