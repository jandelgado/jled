CFLAGS=-std=c++11 -c -Wall -I. -I../src --coverage -fno-inline -fno-inline-small-functions -fno-default-inline -O0 -g
LDFLAGS=-fprofile-arcs -ftest-coverage 
SOURCES=Arduino.cpp test_jled.cpp ../src/jled.cpp 
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=test_jled

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@

coverage: test
	lcov --config-file=.lcovrc --directory ../src --directory .. --capture --output-file coverage.info --no-external
	lcov --config-file=.lcovrc --list coverage.info
	mkdir -p report
	genhtml coverage.info -o report

test: all
	./test_jled

.cpp.o:
	$(CXX) $(CFLAGS) $< -o $@

clean:
	rm -f coverage.info *.{gcov,gcda,gcno,o} ../src/*.{gcov,gcda,gcno,o} $(EXECUTABLE)
