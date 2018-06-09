# use this makefile to build with platformio 
#
.PHONY: all clean upload monitor lint test ci

CIOPTS=--board=uno --board=esp01 --lib="src"

all:
	pio run

lint:
	cpplint --extensions=cpp,h,ino $(shell find .  \( ! -regex '.*/\..*' \) \
		       -type f -a \( -name "*\.cpp" -o -name "*\.h" -o -name "*\.ino" \) )

ci:
	platformio ci  examples/hello/hello.ino $(CIOPTS)
	platformio ci  examples/breathe/breathe.ino $(CIOPTS)
	platformio ci  examples/simple_on/simple_on.ino $(CIOPTS)
	platformio ci  examples/fade_on/fade_on.ino $(CIOPTS)
	platformio ci  examples/fade_off/fade_off.ino $(CIOPTS)
	platformio ci  examples/user_func/user_func.ino $(CIOPTS)
	platformio ci  examples/multiled/multiled.ino $(CIOPTS)

clean:
	pio run --target clean
	rm -f {test,src}/{*.o,*.gcno,*.gcda}

upload:
	pio run --target upload 

monitor:
	pio device monitor 

test:
	$(MAKE) -C test coverage

tags:
	ctags -R
