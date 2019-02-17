# use this makefile to build with platformio 
#
.PHONY: all clean upload monitor lint test ci

# some of the examples use LED_BUILTIN which is not defined for ESP32
CIOPTS=--board=uno --board=esp01 --lib="src"
CIOPTSALL=--board=esp32dev --board=uno --board=esp01 --lib="src"

all:
	pio run

lint:
	cpplint --extensions=cpp,h,ino $(shell find .  \( ! -regex '.*/\..*' \) \
		       -type f -a \( -name "*\.cpp" -o -name "*\.h" -o -name "*\.ino" \) )

ci:
	platformio ci $(CIOPTS) --lib="examples/morse" examples/morse/morse.ino 
	platformio ci $(CIOPTS) examples/multiled/multiled.ino 
	platformio ci $(CIOPTS) examples/user_func/user_func.ino 
	platformio ci $(CIOPTS) examples/hello/hello.ino 
	platformio ci $(CIOPTSALL) examples/breathe/breathe.ino
	platformio ci $(CIOPTS) examples/simple_on/simple_on.ino
	platformio ci $(CIOPTSALL) examples/fade_on/fade_on.ino
	platformio ci $(CIOPTSALL) examples/sequence/sequence.ino 

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
	$(MAKE) -C test coverage

tags:
	ctags -R
