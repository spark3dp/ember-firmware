#MODEL=uno
MODEL=pro328
#PORT=/dev/tty.usbmodem1411
PORT=/dev/tty.usbserial-AH00ZSKO

all: build upload serial

build: src/*
	ino build -m ${MODEL}

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

