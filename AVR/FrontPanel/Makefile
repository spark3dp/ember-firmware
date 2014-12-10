#MODEL=uno
MODEL=pro328
#PORT=/dev/tty.usbmodem1411
PORT=/dev/tty.usbserial-A907ZP1H
DIST_DIR=../../
PROJECT=AUTO-CONTROL-PANEL
REVISION=A.3
VERSION=$(shell cat .version)
NEW_VERSION=$(shell echo $(VERSION) + 1 | bc)
PREV_COMMIT_ID=$(shell git rev-parse HEAD~1 | cut -c1-10)
COMMIT_ID=$(shell git rev-parse HEAD | cut -c1-10)

all: build upload serial

build: src/*
	ino build -m ${MODEL}

upload:
	ino upload 

wait:
	sleep 2

uploadp:
	ino upload -p ${PORT} 

serial:
	ino serial 

serialp:
	ino serial -p ${PORT}

docs:
	doxygen doxyconf

dist:
	@cd ${DIST_DIR};rm -f "[${PROJECT}-${REVISION}-${PREV_COMMIT_ID}] firmware.zip"
	@cd ${DIST_DIR};rm -f "[${PROJECT}-${REVISION}-${COMMIT_ID}] firmware.zip"
	@cd ${DIST_DIR}; zip -r "[${PROJECT}-${REVISION}-${COMMIT_ID}] firmware.zip" firmware

prog: build
	@rm -f firmware.hex
	@ln -s .build/${MODEL}/firmware.hex .
	avrdude -pm328p -F -cusbtiny -V -Uflash:w:firmware.hex:i

bump_version:
	@echo "Current Version: $(REVISION).$(VERSION)"
	@echo "New Version: $(REVISION).$(NEW_VERSION)"
	@echo $(NEW_VERSION) > .version

clean:
	ino clean

.PHONY: docs upload serial clean wait dist bump_version
