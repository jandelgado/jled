CC=g++
CFLAGS=-std=c++11 -c -Wall -I../src -I. -fno-implement-inlines -fno-inline -fno-inline-small-functions --coverage -O0
LDFLAGS=-fprofile-arcs #-ftest-coverage 
SOURCES=Arduino.cpp test_jled.cpp ../src/jled.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=test_jled

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

test: all
	./test_jled
	cd .. && gcovr  -r . --exclude "test/.*"
	cd .. && gcovr  -r . --branches --exclude "test/.*"

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f *.{gcda,gcno,o} ../src/*.{gcda,gcno,o} $(EXECUTABLE)
