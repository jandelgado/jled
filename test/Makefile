# JLed unit tests Makefile
# run `make coverage` to run all test and calculate coverage
#
.PHONY: phony

CFLAGS=-std=c++14 -c -Wall -Wextra -I. -I../src -I./esp-idf \
	   --coverage -fno-inline \
	   -fno-inline-small-functions -fno-default-inline -g -fmax-errors=5 \
	   -fno-omit-frame-pointer -fno-optimize-sibling-calls \
	   $(OPT)

LDFLAGS=-fprofile-arcs -ftest-coverage

CATCH=catch2/catch_amalgamated.cpp

TEST_ARDUINO_MOCK_SRC=Arduino.cpp test_arduino_mock.cpp test_main.cpp ${CATCH}
TEST_ARDUINO_MOCK_OBJECTS=$(TEST_ARDUINO_MOCK_SRC:.cpp=.o)

TEST_JLED_SRC=Arduino.cpp test_jled.cpp test_main.cpp ../src/jled_base.cpp ${CATCH}
TEST_JLED_OBJECTS=$(TEST_JLED_SRC:.cpp=.o)

TEST_JLED_SEQUENCE_SRC=Arduino.cpp test_jled_sequence.cpp test_main.cpp ../src/jled_base.cpp ${CATCH}
TEST_JLED_SEQUENCE_OBJECTS=$(TEST_JLED_SEQUENCE_SRC:.cpp=.o)

TEST_ESP32_SRC=esp-idf/esp_timer.cpp esp-idf/driver/ledc.cpp \
			   test_esp32_hal.cpp ../src/esp32_hal.cpp test_main.cpp ${CATCH}
TEST_ESP32_OBJECTS=$(TEST_ESP32_SRC:.cpp=.o)

TEST_MBED_SRC=mbed.cpp test_mbed_hal.cpp test_main.cpp ${CATCH}
TEST_MBED_OBJECTS=$(TEST_MBED_SRC:.cpp=.o)

TEST_ARDUINO_SRC=Arduino.cpp test_arduino_hal.cpp test_main.cpp ${CATCH}
TEST_ARDUINO_OBJECTS=$(TEST_ARDUINO_SRC:.cpp=.o)

TEST_MORSE_SRC=test_example_morse.cpp test_main.cpp  ${CATCH}
TEST_MORSE_OBJECTS=$(TEST_MORSE_SRC:.cpp=.o)


all: bin test_arduino_mock  \
	 test_jled test_jled_sequence \
	 test_esp32_hal test_arduino_hal test_mbed_hal \
	 test_example_morse

bin/test_arduino_mock: $(TEST_ARDUINO_MOCK_OBJECTS)
	$(CXX) -o $@ $(LDFLAGS) $(TEST_ARDUINO_MOCK_OBJECTS)

bin/test_jled: $(TEST_JLED_OBJECTS)
	$(CXX) -o $@ $(LDFLAGS) $(TEST_JLED_OBJECTS)

bin/test_jled_sequence: $(TEST_JLED_SEQUENCE_OBJECTS)
	$(CXX) -o $@ $(LDFLAGS) $(TEST_JLED_SEQUENCE_OBJECTS)

bin/test_esp32_hal: CFLAGS += -DESP32
bin/test_esp32_hal: $(TEST_ESP32_OBJECTS)
	$(CXX) -o $@ $(LDFLAGS) $(TEST_ESP32_OBJECTS)

bin/test_mbed_hal: CFLAGS += -D__MBED__
bin/test_mbed_hal: $(TEST_MBED_OBJECTS)
	$(CXX) -o $@ $(LDFLAGS) $(TEST_MBED_OBJECTS)

bin/test_arduino_hal: $(TEST_ARDUINO_OBJECTS)
	$(CXX) -o $@ $(LDFLAGS) $(TEST_ARDUINO_OBJECTS)

bin/test_example_morse: CFLAGS += -I../examples/morse
bin/test_example_morse: $(TEST_MORSE_OBJECTS)
	$(CXX) -o $@ $(LDFLAGS) $(TEST_MORSE_OBJECTS)

coverage: test
	lcov --config-file=.lcovrc --directory ../src --directory .. --capture --output-file coverage.lcov --no-external
	lcov --config-file=.lcovrc --list coverage.lcov
	mkdir -p report
	genhtml coverage.lcov -o report

test: depend all

test_arduino_mock: ./bin/test_arduino_mock
	./bin/test_arduino_mock $(ARGS)

test_jled: ./bin/test_jled
	-./bin/test_jled $(ARGS)

test_jled_sequence: ./bin/test_jled_sequence
	./bin/test_jled_sequence $(ARGS)

test_esp32_hal:	./bin/test_esp32_hal
	./bin/test_esp32_hal $(ARGS)

test_mbed_hal:	./bin/test_mbed_hal
	./bin/test_mbed_hal $(ARGS)

test_arduino_hal: ./bin/test_arduino_hal
	./bin/test_arduino_hal $(ARGS)

test_example_morse: ./bin/test_example_morse
	./bin/test_example_morse $(ARGS)


.cpp.o:
	$(CXX) $< $(CFLAGS) -o $@

.hpp.pch:
	$(CXX) $< $(CFLAGS) -o $@

bin:
	mkdir -p bin

clean: phony
	rm -f {./,esp-idf,esp-idf/driver,catch2}/{*.gcov,*.gcda,*.gcno,*.o} .depend

clobber: clean
	rm -f bin/*

depend: .depend

.depend: $(TEST_JLED_SRC) $(TEST_JLED_SEQUENCE_SRC) $(TEST_ESP32_SRC) $(TEST_ARDUINO_SRC) $(TEST_MBED_SRC) $(TEST_MORSE_SRC)
	@echo updating dependencies in .depend
	@rm -f ./.depend
	@$(CC) -I ../src -I .  -MM $^ > .depend

include .depend

