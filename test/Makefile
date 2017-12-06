CC=g++
CFLAGS=-std=c++11 -c -Wall -I. -I../src --coverage -fno-inline -fno-inline-small-functions -fno-default-inline -O0 -g
LDFLAGS=-fprofile-arcs -ftest-coverage 
SOURCES=Arduino.cpp test_jled.cpp ../src/jled.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=test_jled

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

coverage: test
	lcov --config-file=.lcovrc --directory .. --capture --output-file coverage.info
	lcov --config-file=.lcovrc --remove coverage.info 'test/*' '/usr/*' --output-file coverage.info
	lcov --config-file=.lcovrc --list coverage.info

test: all
	./test_jled

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f coverage.info *.{gcov,gcda,gcno,o} ../src/*.{gcov,gcda,gcno,o} $(EXECUTABLE)
