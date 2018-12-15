# JLed unit tests Makefile
# run `make coverage` to run all test and calculate coverage
CFLAGS=-std=c++11 -c -Wall -Wextra -I. -I../src --coverage -fno-inline \
	   -fno-inline-small-functions -fno-default-inline -g -fmax-errors=5 \
	   -fno-omit-frame-pointer -fno-optimize-sibling-calls $(OPT)

LDFLAGS=-fprofile-arcs -ftest-coverage 

TEST_JLED_SOURCES=Arduino.cpp test_jled.cpp test_mock.cpp test_main.cpp ../src/jled_base.cpp
TEST_JLED_OBJECTS=$(TEST_JLED_SOURCES:.cpp=.o) 

TEST_JLED_SEQUENCE_SOURCES=Arduino.cpp test_jled_sequence.cpp test_mock.cpp test_main.cpp ../src/jled_base.cpp
TEST_JLED_SEQUENCE_OBJECTS=$(TEST_JLED_SEQUENCE_SOURCES:.cpp=.o)

TEST_ESP32_SOURCES=Arduino.cpp test_esp32_hal.cpp ../src/esp32_hal.cpp test_main.cpp
TEST_ESP32_OBJECTS=$(TEST_ESP32_SOURCES:.cpp=.o)

TEST_ESP8266_SOURCES=Arduino.cpp test_esp8266_hal.cpp test_main.cpp
TEST_ESP8266_OBJECTS=$(TEST_ESP8266_SOURCES:.cpp=.o)

TEST_ARDUINO_SOURCES=Arduino.cpp test_arduino_hal.cpp test_main.cpp
TEST_ARDUINO_OBJECTS=$(TEST_ARDUINO_SOURCES:.cpp=.o)

all: bin bin/test_jled bin/test_jled_sequence bin/test_esp32_hal bin/test_esp8266_hal bin/test_arduino_hal

bin/test_jled: $(TEST_JLED_OBJECTS)
	$(CXX) $(LDFLAGS) $(TEST_JLED_OBJECTS) -o $@

bin/test_jled_sequence: $(TEST_JLED_SEQUENCE_OBJECTS)
	$(CXX) $(LDFLAGS) $(TEST_JLED_SEQUENCE_OBJECTS) -o $@

bin/test_esp32_hal: CFLAGS += -DESP32
bin/test_esp32_hal: $(TEST_ESP32_OBJECTS)
	$(CXX) $(LDFLAGS) $(TEST_ESP32_OBJECTS) -o $@

bin/test_esp8266_hal: $(TEST_ESP8266_OBJECTS)
	$(CXX) $(LDFLAGS) $(TEST_ESP8266_OBJECTS) -o $@

bin/test_arduino_hal: $(TEST_ARDUINO_OBJECTS)
	$(CXX) $(LDFLAGS) $(TEST_ARDUINO_OBJECTS) -o $@


coverage: test
	lcov --config-file=.lcovrc --directory ../src --directory .. --capture --output-file coverage.info --no-external
	lcov --config-file=.lcovrc --list coverage.info
	mkdir -p report
	genhtml coverage.info -o report

test: all
	./bin/test_jled
	./bin/test_jled_sequence
	./bin/test_esp32_hal
	./bin/test_esp8266_hal
	./bin/test_arduino_hal

.cpp.o:
	$(CXX) $(CFLAGS) $< -o $@

.hpp.pch:
	$(CXX) $(CFLAGS) $< -o $@

bin:
	mkdir -p bin

clean:
	rm -f coverage.info *.{gcov,gcda,gcno,o} ../src/*.{gcov,gcda,gcno,o} 

clobber: clean
	rm -f bin/*

