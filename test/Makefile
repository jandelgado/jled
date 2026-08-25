# JLed unit tests Makefile
# run `make coverage` to run all test and calculate coverage
#
.PHONY: phony

CFLAGS=-std=c++14 -c -Wall -Wextra -I. -I../src -I./esp-idf \
	   --coverage -fno-inline \
	   -O1\
	   -fno-inline-small-functions -fno-default-inline -g -fmax-errors=5 \
	   -fno-omit-frame-pointer -fno-optimize-sibling-calls \
	   $(OPT)

LDFLAGS=-fprofile-arcs -ftest-coverage

CATCH=catch2/catch_amalgamated.cpp

TEST_ARDUINO_MOCK_SRC=Arduino.cpp test_arduino_mock.cpp test_main.cpp ${CATCH}
TEST_ARDUINO_MOCK_OBJECTS=$(TEST_ARDUINO_MOCK_SRC:.cpp=.o)

TEST_JLED_SRC=Arduino.cpp test_jled.cpp test_main.cpp ../src/jled_effects.cpp ${CATCH}
TEST_JLED_OBJECTS=$(TEST_JLED_SRC:.cpp=.o)

TEST_ESP32_SRC=esp-idf/esp_timer.cpp esp-idf/driver/ledc.cpp \
			   esp32_mock.cpp \
			   test_esp32_hal.cpp test_main.cpp ${CATCH}
TEST_ESP32_OBJECTS=$(TEST_ESP32_SRC:.cpp=.o)

TEST_ESP32_LEGACY_SRC=esp-idf/esp_timer.cpp esp-idf/driver/ledc.cpp \
			   esp32_mock.cpp \
			   test_esp32_hal_legacy_idf.cpp test_main.cpp ${CATCH}
TEST_ESP32_LEGACY_OBJECTS=$(TEST_ESP32_LEGACY_SRC:.cpp=.o)

TEST_MBED_SRC=mbed.cpp test_mbed_hal.cpp test_main.cpp ${CATCH}
TEST_MBED_OBJECTS=$(TEST_MBED_SRC:.cpp=.o)

TEST_ARDUINO_SRC=Arduino.cpp test_arduino_hal.cpp test_main.cpp ${CATCH}
TEST_ARDUINO_OBJECTS=$(TEST_ARDUINO_SRC:.cpp=.o)

TEST_INVERTABLE_HAL_SRC=Arduino.cpp test_invertable_hal.cpp test_main.cpp ${CATCH}
TEST_INVERTABLE_HAL_OBJECTS=$(TEST_INVERTABLE_HAL_SRC:.cpp=.o)

TEST_MORSE_SRC=test_example_morse.cpp test_main.cpp  ${CATCH}
TEST_MORSE_OBJECTS=$(TEST_MORSE_SRC:.cpp=.o)

TEST_LIGHT_SCALAR_SRC=test_value_scalar.cpp test_main.cpp ${CATCH}
TEST_LIGHT_SCALAR_OBJECTS=$(TEST_LIGHT_SCALAR_SRC:.cpp=.o)

TEST_SCALE_BIT_DEPTH_SRC=test_scale_bit_depth.cpp test_main.cpp ${CATCH}
TEST_SCALE_BIT_DEPTH_OBJECTS=$(TEST_SCALE_BIT_DEPTH_SRC:.cpp=.o)

TEST_SCALE_LERP_SRC=test_scale_lerp.cpp test_main.cpp ${CATCH}
TEST_SCALE_LERP_OBJECTS=$(TEST_SCALE_LERP_SRC:.cpp=.o)

TEST_LIGHT_HSV_SRC=test_value_hsv.cpp test_main.cpp ${CATCH}
TEST_LIGHT_HSV_OBJECTS=$(TEST_LIGHT_HSV_SRC:.cpp=.o)

TEST_LIGHT_RGB_SRC=test_value_rgb.cpp test_main.cpp ../src/value_rgb.cpp ${CATCH}
TEST_LIGHT_RGB_OBJECTS=$(TEST_LIGHT_RGB_SRC:.cpp=.o)

TEST_FADEON_SRC=test_fadeon_func.cpp test_main.cpp ../src/jled_effects.cpp ${CATCH}
TEST_FADEON_OBJECTS=$(TEST_FADEON_SRC:.cpp=.o)

TEST_PICO_SRC=pico_mock.cpp test_pico_hal.cpp test_main.cpp ${CATCH}
TEST_PICO_OBJECTS=$(TEST_PICO_SRC:.cpp=.o)

TEST_STM32CUBE_SRC=stm32cube_hal_mock.cpp test_stm32cube_hal.cpp test_main.cpp ${CATCH}
TEST_STM32CUBE_OBJECTS=$(TEST_STM32CUBE_SRC:.cpp=.o)

TEST_ESP8266_HAL_SRC=test_jled_esp8266_hal.cpp test_main.cpp ${CATCH}
TEST_ESP8266_HAL_OBJECTS=$(TEST_ESP8266_HAL_SRC:.cpp=.o)

TEST_JLED_GROUP_SRC=test_jled_group.cpp test_main.cpp ../src/jled_effects.cpp \
					../src/value_rgb.cpp ${CATCH}
TEST_JLED_GROUP_OBJECTS=$(TEST_JLED_GROUP_SRC:.cpp=.o)

TEST_RGB_HAL_SRC=test_rgb_hal.cpp test_main.cpp ${CATCH}
TEST_RGB_HAL_OBJECTS=$(TEST_RGB_HAL_SRC:.cpp=.o)

TEST_WRITER_HAL_SRC=test_writer_hal.cpp test_main.cpp ${CATCH}
TEST_WRITER_HAL_OBJECTS=$(TEST_WRITER_HAL_SRC:.cpp=.o)

TEST_JLED_RGB_SRC=Arduino.cpp test_jled_rgb.cpp test_main.cpp ../src/jled_effects.cpp \
				  ../src/value_rgb.cpp ${CATCH}
TEST_JLED_RGB_OBJECTS=$(TEST_JLED_RGB_SRC:.cpp=.o)

TEST_JLED_RGB_HD_SRC=Arduino.cpp test_jled_rgb_hd.cpp test_main.cpp ../src/jled_effects.cpp \
					 ../src/value_rgb.cpp ${CATCH}
TEST_JLED_RGB_HD_OBJECTS=$(TEST_JLED_RGB_HD_SRC:.cpp=.o)

TEST_COLOR_PALETTE_SRC=test_color_palette.cpp test_main.cpp ../src/value_rgb.cpp ${CATCH}
TEST_COLOR_PALETTE_OBJECTS=$(TEST_COLOR_PALETTE_SRC:.cpp=.o)

all: bin bin/test_arduino_mock  \
	 bin/test_jled \
	 bin/test_esp32_hal bin/test_esp32_hal_legacy_idf bin/test_arduino_hal bin/test_mbed_hal \
	 bin/test_pico_hal bin/test_jled_esp8266_hal \
	 bin/test_example_morse bin/test_value_scalar bin/test_scale_bit_depth bin/test_scale_lerp \
	 bin/test_fadeon_func \
	 bin/test_invertable_hal bin/test_jled_group bin/test_stm32cube_hal \
	 bin/test_value_hsv bin/test_value_rgb bin/test_rgb_hal bin/test_writer_hal \
	 bin/test_jled_rgb bin/test_jled_rgb_hd bin/test_color_palette

bin/test_arduino_mock: $(TEST_ARDUINO_MOCK_OBJECTS)
	$(CXX) -o $@ $(LDFLAGS) $(TEST_ARDUINO_MOCK_OBJECTS)

bin/test_jled: $(TEST_JLED_OBJECTS)
	$(CXX) -o $@ $(LDFLAGS) $(TEST_JLED_OBJECTS)

bin/test_esp32_hal: CFLAGS += -DESP32
bin/test_esp32_hal: $(TEST_ESP32_OBJECTS)
	$(CXX) -o $@ $(LDFLAGS) $(TEST_ESP32_OBJECTS)

bin/test_esp32_hal_legacy_idf: CFLAGS += -DESP32
bin/test_esp32_hal_legacy_idf: $(TEST_ESP32_LEGACY_OBJECTS)
	$(CXX) -o $@ $(LDFLAGS) $(TEST_ESP32_LEGACY_OBJECTS)

bin/test_mbed_hal: CFLAGS += -D__MBED__
bin/test_mbed_hal: $(TEST_MBED_OBJECTS)
	$(CXX) -o $@ $(LDFLAGS) $(TEST_MBED_OBJECTS)

bin/test_arduino_hal: $(TEST_ARDUINO_OBJECTS)
	$(CXX) -o $@ $(LDFLAGS) $(TEST_ARDUINO_OBJECTS)

bin/test_invertable_hal: $(TEST_INVERTABLE_HAL_OBJECTS)
	$(CXX) -o $@ $(LDFLAGS) $(TEST_INVERTABLE_HAL_OBJECTS)

bin/test_pico_hal: CFLAGS += -I./pico-sdk
bin/test_pico_hal: $(TEST_PICO_OBJECTS)
	$(CXX) -o $@ $(LDFLAGS) $(TEST_PICO_OBJECTS)

bin/test_jled_esp8266_hal: $(TEST_ESP8266_HAL_OBJECTS)
	$(CXX) -o $@ $(LDFLAGS) $(TEST_ESP8266_HAL_OBJECTS)

bin/test_example_morse: CFLAGS += -I../examples/morse
bin/test_example_morse: $(TEST_MORSE_OBJECTS)
	$(CXX) -o $@ $(LDFLAGS) $(TEST_MORSE_OBJECTS)

bin/test_value_scalar: $(TEST_LIGHT_SCALAR_OBJECTS)
	$(CXX) -o $@ $(LDFLAGS) $(TEST_LIGHT_SCALAR_OBJECTS)

bin/test_scale_bit_depth: $(TEST_SCALE_BIT_DEPTH_OBJECTS)
	$(CXX) -o $@ $(LDFLAGS) $(TEST_SCALE_BIT_DEPTH_OBJECTS)

bin/test_scale_lerp: $(TEST_SCALE_LERP_OBJECTS)
	$(CXX) -o $@ $(LDFLAGS) $(TEST_SCALE_LERP_OBJECTS)

bin/test_fadeon_func: $(TEST_FADEON_OBJECTS)
	$(CXX) -o $@ $(LDFLAGS) $(TEST_FADEON_OBJECTS)

bin/test_value_hsv: $(TEST_LIGHT_HSV_OBJECTS)
	$(CXX) -o $@ $(LDFLAGS) $(TEST_LIGHT_HSV_OBJECTS)

bin/test_value_rgb: $(TEST_LIGHT_RGB_OBJECTS)
	$(CXX) -o $@ $(LDFLAGS) $(TEST_LIGHT_RGB_OBJECTS)

bin/test_jled_group: $(TEST_JLED_GROUP_OBJECTS)
	$(CXX) -o $@ $(LDFLAGS) $(TEST_JLED_GROUP_OBJECTS)

bin/test_stm32cube_hal: $(TEST_STM32CUBE_OBJECTS)
	$(CXX) -o $@ $(LDFLAGS) $(TEST_STM32CUBE_OBJECTS)

bin/test_rgb_hal: $(TEST_RGB_HAL_OBJECTS)
	$(CXX) -o $@ $(LDFLAGS) $(TEST_RGB_HAL_OBJECTS)

bin/test_writer_hal: $(TEST_WRITER_HAL_OBJECTS)
	$(CXX) -o $@ $(LDFLAGS) $(TEST_WRITER_HAL_OBJECTS)

bin/test_jled_rgb: $(TEST_JLED_RGB_OBJECTS)
	$(CXX) -o $@ $(LDFLAGS) $(TEST_JLED_RGB_OBJECTS)

bin/test_jled_rgb_hd: $(TEST_JLED_RGB_HD_OBJECTS)
	$(CXX) -o $@ $(LDFLAGS) $(TEST_JLED_RGB_HD_OBJECTS)

bin/test_color_palette: $(TEST_COLOR_PALETTE_OBJECTS)
	$(CXX) -o $@ $(LDFLAGS) $(TEST_COLOR_PALETTE_OBJECTS)

coverage: test
	mkdir -p report
	gcovr --root .. --filter ../src/ --merge-lines --merge-mode-functions=merge-use-line-max --print-summary --txt --coveralls coverage.json --html-details report/index.html --lcov report.info

test: depend all
	./bin/test_arduino_mock
	./bin/test_jled
	./bin/test_mbed_hal
	./bin/test_esp32_hal
	./bin/test_esp32_hal_legacy_idf
	./bin/test_arduino_hal
	./bin/test_pico_hal
	./bin/test_jled_esp8266_hal
	./bin/test_example_morse
	./bin/test_value_scalar
	./bin/test_scale_bit_depth
	./bin/test_scale_lerp
	./bin/test_value_hsv
	./bin/test_value_rgb
	./bin/test_fadeon_func
	./bin/test_invertable_hal
	./bin/test_jled_group
	./bin/test_stm32cube_hal
	./bin/test_rgb_hal
	./bin/test_writer_hal
	./bin/test_jled_rgb
	./bin/test_jled_rgb_hd
	./bin/test_color_palette

.cpp.o:
	$(CXX) $< $(CFLAGS) -o $@

bin:
	mkdir -p bin

clean: phony
	rm -f {./,esp-idf,esp-idf/driver,catch2}/{*.gcov,*.gcda,*.gcno,*.o} .depend
	rm -f ../src/{*.o,*.gcno,*.gcda}

clobber: clean
	rm -f bin/*

depend: .depend

.depend: $(TEST_ARDUINO_MOCK_SRC) $(TEST_JLED_SRC) $(TEST_ESP32_SRC) $(TEST_ESP32_LEGACY_SRC) $(TEST_ESP8266_HAL_SRC) $(TEST_ARDUINO_SRC) $(TEST_INVERTABLE_HAL_SRC) $(TEST_MBED_SRC) $(TEST_PICO_SRC) $(TEST_MORSE_SRC) $(TEST_LIGHT_SCALAR_SRC) $(TEST_SCALE_BIT_DEPTH_SRC) $(TEST_SCALE_LERP_SRC) $(TEST_LIGHT_HSV_SRC) $(TEST_LIGHT_RGB_SRC) $(TEST_FADEON_SRC) $(TEST_JLED_GROUP_SRC) $(TEST_STM32CUBE_SRC) $(TEST_RGB_HAL_SRC) $(TEST_JLED_RGB_SRC) $(TEST_JLED_RGB_HD_SRC) $(TEST_COLOR_PALETTE_SRC) $(TEST_WRITER_HAL)
	@echo updating dependencies in .depend
	@rm -f ./.depend
	@$(CC) -I ../src -I . -I ./pico-sdk -MM $^ > .depend

include .depend

