SHELL = /bin/bash

.DELETE_ON_ERROR:
.PHONY: build load clean

build:
	@arduino-cli compile --build-path ./build --fqbn \
	arduino:avr:nano:cpu=atmega328 ./arduino.ino

load:
	@arduino-cli upload --input-dir ./build -p $(PORT) --fqbn \
	arduino:avr:nano:cpu=atmega328 ./arduino.ino

clean:
	@-rm -rf ./build
