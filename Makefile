# use this makefile to build with platformio 
#
.PHONY: all clean upload monitor lint test ci

CIOPTS=--board=uno --board=esp01 --lib="src"
CIOPTS_ESP32=--board=esp32dev --lib="src"

all:
	pio run

lint:
	cpplint --extensions=cpp,h,ino $(shell find .  \( ! -regex '.*/\..*' \) \
		       -type f -a \( -name "*\.cpp" -o -name "*\.h" -o -name "*\.ino" \) )

ci:
	platformio ci $(CIOPTS) examples/multiled/multiled.ino 
	platformio ci $(CIOPTS_ESP32) examples/multiled_esp32/multiled_esp32.ino src/esp32_hal.cpp 
	platformio ci $(CIOPTS) examples/user_func/user_func.ino 
	platformio ci $(CIOPTS) examples/hello/hello.ino 
	platformio ci $(CIOPTS) examples/breathe/breathe.ino
	platformio ci $(CIOPTS) examples/simple_on/simple_on.ino
	platformio ci $(CIOPTS) examples/fade_on/fade_on.ino
	platformio ci $(CIOPTS) examples/fade_off/fade_off.ino 

clean:
	-pio run --target clean
	rm -f {test,src}/{*.o,*.gcno,*.gcda}

upload:
	pio run --target upload 

monitor:
	pio device monitor 

test:
	$(MAKE) -C test coverage

tags:
	ctags -R
