# use this makefile to build with platformio
#
.PHONY: all clean upload monitor lint test ci

# some of the examples use LED_BUILTIN which is not defined for ESP32
CIOPTS=--board=uno --board=esp01 --board=nano33ble --lib="src"
CIOPTS_MBED=--board=nucleo_f401re -Oframework=mbed --lib="src"
CIOPTSALL=--board=esp32dev --board=uno --board=nano33ble --board=esp01 --lib="src"

all:
	pio run

lint:
	cpplint --extensions=cpp,h,ino $(shell find .  -maxdepth 3 \( ! -regex '.*/\..*' \) \
		       -type f -a \( -name "*\.cpp" -o -name "*\.h" -o -name "*\.ino" \) )

ci:
	pio ci $(CIOPTS) examples/custom_hal/custom_hal.ino
	pio ci $(CIOPTS_MBED) examples/multiled_mbed/multiled_mbed.cpp
	pio ci $(CIOPTS) --lib="examples/morse" examples/morse/morse.ino
	pio ci $(CIOPTS) examples/candle/candle.ino
	pio ci $(CIOPTS) examples/multiled/multiled.ino
	pio ci $(CIOPTS) examples/user_func/user_func.ino
	pio ci $(CIOPTS) examples/hello/hello.ino
	pio ci $(CIOPTSALL) examples/breathe/breathe.ino
	pio ci $(CIOPTS) examples/simple_on/simple_on.ino
	pio ci $(CIOPTSALL) examples/fade_on/fade_on.ino
	pio ci $(CIOPTSALL) examples/sequence/sequence.ino

envdump:
	-pio run --target envdump

clean:
	-pio run --target clean
	rm -f {test,src}/{*.o,*.gcno,*.gcda}

upload:
	pio run --target upload

monitor:
	pio device monitor

test:
	$(MAKE) -C test coverage OPT=-O0

tags:
	ctags -R
