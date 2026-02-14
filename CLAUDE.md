# JLed LED library for embedded devices

Summary: JLed is an embedded C++ library to control LEDs. It uses a non-blocking approach and can
control LEDs in simple (on/off) and complex (blinking,
breathing and more) ways in a time-driven manner. LEDs can be grouped and controlled in
parallel or sequentially.

## Quick Reference

* **Language**: C++14 or later, embedded-friendly (no exceptions, RTTI, or dynamic allocation)
* **Build System**: PlatformIO + Make, managed via devbox
* **Key Constraint**: Non-blocking, time-driven approach for resource-constrained MCUs

## Repository Structure

* `README.md` - User-facing documentation
* `doc/` - Images for documentation
* `examples/` - End-to-end examples for MCU (`.ino` sketches)
* `src/` - Library source code (`.h` and `.cpp` files)
* `test/` - Host-based unit tests with separate Makefile
* `.github/workflows/` - CI/CD configuration
* `devbox.json` - Development environment configuration
* `platformio.ini` - PlatformIO project configuration
* `Makefile` - Top-level build orchestration

## Development Setup

**Prerequisites:**
* [Devbox](https://www.jetify.com/devbox) for dependency management

**Initial setup:**
```bash
devbox shell  # Sets up all dependencies: Python 3.13, lcov 1.16, cpplint 2.0.0, pip, platformio 6.1.18
```

All development tools are configured in `devbox.json` and activated automatically.

## Code Style & Conventions

**Formatting:**
* Code formatting is defined in `.clang-format` (based on Google style)
* Validate with cpplint: `make lint`

**Naming Conventions:**
* Classes/Types: `PascalCase` (e.g., `BrightnessEvaluator`, `JLed`)
* Public methods: `PascalCase` (e.g., `Breathe()`, `Update()`, `FadeOn()`)
* Private members: `snake_case_` with trailing underscore (e.g., `val_`, `duration_on_`)
* Constants: `kPascalCase` (e.g., `kFullBrightness`, `kRepeatForever`)
* Template parameters: Single uppercase letter (e.g., `T`, `B`) or descriptive `PascalCase`

**C++ Guidelines:**
* Use C++14 features
* Prefer `constexpr` over `#define` for constants
* Use `= delete` for disabled constructors
* Use `override` keyword for virtual method overrides
* No floating-point in core logic (expensive on MCUs without FPU)
* Prefer templates over virtual functions for performance-critical code

## Build Instructions

During development we use PlatformIO. All tasks are orchestrated by Makefiles:

### Top-Level Makefile Targets

* `make lint` - Lint all C++ files using cpplint
* `make ci` - Compile all examples for different target platforms (usually run in CI, takes ~10 minutes)
* `make envdump` - Run `pio run --target envdump` to dump detailed PlatformIO environment information
* `make clean` - Clean up all build artifacts
* `make upload` - Upload the active sketch in `platformio.ini` to the configured MCU
* `make monitor` - Start serial monitoring for the configured MCU (for debugging)
* `make test` - Run all unit tests in the `test/` directory and calculate coverage
* `make tags` - Create ctags for all files in the project

### Test Makefile Targets

The tests in `test/` are orchestrated by a separate Makefile, which is called by the top-level
Makefile. `test/Makefile` has the following targets:

* `make test` - Build and run all tests
* `make clean` - Delete all intermediary files (e.g., object files)
* `make clobber` - Like `make clean` plus delete all created test binaries
* `make coverage` - Run tests and calculate coverage (generates HTML report in `test/report/`)

## Testing Guidelines

**Testing Framework:**
* Uses [Catch2](https://github.com/catchorg/Catch2) (amalgamated version in `test/catch2/`)
* Test files: `test/test_*.cpp`
* Common main: `test/test_main.cpp` (contains `CATCH_CONFIG_MAIN`)

**Test Structure:**
* Test naming: `TEST_CASE("description of what is tested", "[tag]")`
* Use `SECTION()` for related test variations within a TEST_CASE
* Tags help organize tests: `[jled]`, `[sequence]`, `[hal]`, etc.

**Writing Tests:**
* All core logic should have unit tests
* Aim for high coverage (check with `make coverage`)
* Test effect evaluators by calling `Eval(t)` at different time points
* Use mocks for HAL testing (see `test/Arduino.h`, `test/esp-idf/` for examples)
* HAL code is primarily tested via integration examples on real hardware

**Adding Tests:**
* When adding a new effect: Add tests in `test/test_jled.cpp`
* When adding HAL support: Add tests in `test/test_[platform]_hal.cpp`
* When fixing a bug: Add a test that reproduces the bug first
* Update `test/Makefile` if adding new test files

## Architecture

**Supported Platforms:**
* Arduino (AVR, SAMD, etc.)
* ESP32 (with Arduino framework)
* ESP8266 (with Arduino framework)
* ARM mbed (Cortex-M)
* Raspberry Pi Pico

Platform detection is automatic via preprocessor macros in `src/jled.h`.

**Core Components:**

* `src/jled_base.h` - Platform-agnostic core logic
  * `TJLed<HalType, B>` template class - Main LED controller with state machine
  * `BrightnessEvaluator` - Abstract base for all effects
  * Effect implementations: `ConstantBrightnessEvaluator`, `BlinkBrightnessEvaluator`,
    `BreatheBrightnessEvaluator`, `CandleBrightnessEvaluator`
  * `TJLedSequence<JLed, B>` template - Controls multiple LEDs in parallel or sequence
* `src/jled.h` - Platform-specific convenience layer
  * Detects platform via preprocessor macros
  * Selects appropriate HAL
  * Provides `JLed` and `JLedSequence` classes
* `src/*_hal.h` - Hardware abstraction layers (ESP32, ESP8266, Arduino, mbed, Pico)

### Important architecture decisions

**Core principle**: `JLed` core logic (i.e. the state machine), the effects calculation and the
access to the hardware are strictly separated. This keeps complexity low and allows us to a) easily
extend `JLed` and b) effectively unit test almost all parts of `JLed`

### Hardware Abstraction Layer (HAL)

* JLed logic/effects and hardware code are strictly separate
* Each platform has a HAL class in `src/*_hal.h` that implements:
  * `analogWrite(pin, value)` - Write PWM value to GPIO pin
  * `millis()` - Return current MCU time in milliseconds
* Uses **compile-time polymorphism** (templates, not virtual functions) for performance
  * Avoids virtual function table overhead in hot paths
  * Zero-cost abstraction

### Effect Abstraction

* An effect is simply a function that calculates a brightness over time
* Each effect implements the abstract `BrightnessEvaluator` class
* The `BrightnessEvaluator` class has two abstract methods that must be overridden:
  * `Period()` - Returns the period of the effect in milliseconds
  * `Eval(t)` - Calculates the brightness [0-255] for the given point in time `t` (0 ≤ t < Period())
* Effects must be stateless - all state is managed by the `TJLed` class
* Effects should be copyable (used with placement new)

### Memory Management

* Uses **C++ placement new** to avoid dynamic allocation
* Each `JLed` instance has a fixed buffer: `alignas(...) char brightness_eval_buf_[MAX_SIZE]`
* `MAX_SIZE` is compile-time calculated as max size of all effect evaluator types:
  ```cpp
  __max(sizeof(CandleBrightnessEvaluator),
        __max(sizeof(BreatheBrightnessEvaluator),
              __max(sizeof(ConstantBrightnessEvaluator),
                    sizeof(BlinkBrightnessEvaluator))))
  ```
* When effect changes (e.g., `.Fade()`), old evaluator is destroyed, new one constructed in same buffer
* Copy constructor clones evaluator into new `JLed` instance's buffer

### Configuration Using a Fluent Interface

* JLed uses a fluent interface in the public API that allows configuring LEDs in an
  intuitive way, e.g.:
  ```cpp
  JLed led = JLed(21).DelayBefore(1500).Breathe(500).Repeat(5).MaxBrightness(150);
  ```
* Methods return `B&` (reference to derived type) using CRTP pattern for chainability
* This approach allows readable, self-documenting LED configurations

### Known Technical Debt

* The HAL should not contain a `millis()` function since this is a property of the MCU and
  not of a GPIO. Possible fix: refactor the HAL concept to introduce a `TimeHAL` abstraction

## Embedded Constraints & Design Philosophy

**Embedded System Constraints:**
* **No dynamic allocation**: MCUs have limited heap; avoid new/delete in library code
* **No exceptions**: Not available on many embedded platforms
* **No RTTI**: Keep binary size small
* **Timing critical**: `Update()` must be fast - called frequently in main loop
* **Memory footprint**: Keep instance size small (users may control many LEDs)
* **Non-blocking**: Never use `delay()` - always time-driven approach

**Design Philosophy:**
* **Simplicity over features**: Don't add complexity for edge cases
* **Testability**: If it can't be unit tested, reconsider the design
* **Portability**: Core logic must be platform-agnostic
* **Performance**: Critical path (`Update()`/`Eval()`) must be optimized
* **Separation of concerns**: Logic, effects, and hardware access are strictly separated

## Common Development Tasks

### Adding a New Effect

1. **Create the evaluator class** in `src/jled_base.h`:
   ```cpp
   class MyEffectBrightnessEvaluator : public CloneableBrightnessEvaluator {
       uint16_t period_;
    public:
       explicit MyEffectBrightnessEvaluator(uint16_t period) : period_(period) {}
       BrightnessEvaluator* clone(void* ptr) const override {
           return new (ptr) MyEffectBrightnessEvaluator(*this);
       }
       uint16_t Period() const override { return period_; }
       uint8_t Eval(uint32_t t) const override {
           // Calculate brightness [0-255] based on time t
           return /* your calculation */;
       }
   };
   ```

2. **Update MAX_SIZE calculation** if your evaluator is larger than existing ones (in `TJLed` class)

3. **Add fluent interface method** in `TJLed` template class:
   ```cpp
   B& MyEffect(uint16_t period) {
       return SetBrightnessEval(
           new (brightness_eval_buf_) MyEffectBrightnessEvaluator(period));
   }
   ```

4. **Add unit tests** in `test/test_jled.cpp`:
   ```cpp
   TEST_CASE("MyEffect works correctly", "[jled]") {
       // Test the effect evaluator and integration with JLed
   }
   ```

5. **Add an example** in `examples/my_effect/my_effect.ino`

6. **Update README.md** with documentation and example

### Adding HAL Support for New MCU

1. **Create HAL header** `src/[platform]_hal.h`:
   ```cpp
   class MyPlatformHal {
    public:
       using PinType = uint8_t;  // or appropriate type

       MyPlatformHal() = delete;
       explicit MyPlatformHal(PinType pin) : pin_(pin) {
           // Initialize pin for PWM output
       }

       void analogWrite(uint8_t val) {
           // Write PWM value to pin
       }

       uint32_t millis() const {
           // Return current time in milliseconds
       }

    private:
       PinType pin_;
   };
   ```

2. **Add platform detection** in `src/jled.h`:
   ```cpp
   #elif defined(MY_PLATFORM_MACRO)
   #include "my_platform_hal.h"
   namespace jled {using JLedHalType = MyPlatformHal;}
   ```

3. **Add unit tests** in `test/test_my_platform_hal.cpp` with appropriate mocks

4. **Test on actual hardware** using existing examples

5. **Update CI** (if possible) in `.github/workflows/test.yml` to include your platform

6. **Update README.md** and this file to document the new platform

### Adding a New Example

1. **Create directory** `examples/my_example/`

2. **Create sketch** `examples/my_example/my_example.ino`:
   ```cpp
   #include <jled.h>

   // Keep it simple and focused on one concept
   // Add comments explaining what the example demonstrates

   void setup() { }
   void loop() { }
   ```

3. **Test on hardware** - ensure it works on at least one platform

4. **Add to platformio.ini** if needed (comment out by default)

5. **Update README.md** if the example demonstrates a new concept

### Fixing a Bug

1. **Write a failing test** that reproduces the bug in `test/test_*.cpp`

2. **Fix the bug** in the appropriate source file

3. **Verify test passes** with `make test`

4. **Check coverage** with `make coverage` - ensure the fix is covered

5. **Run lint** with `make lint`

6. **Test on hardware** if it's a platform-specific issue

## Do's and Don'ts

**DO:**
* ✓ Preserve backwards compatibility in public API
* ✓ Add tests for all bug fixes and new features
* ✓ Run `make lint` and `make test` before committing
* ✓ Test on actual hardware when possible
* ✓ Document public API in code comments

**DON'T:**
* ✗ Add platform-specific code to `jled_base.h` - use HAL instead
* ✗ Use floating-point in core logic - slow on MCUs without FPU
* ✗ Make breaking changes without major version bump
* ✗ Add features requiring dynamic allocation, exceptions, or RTTI
* ✗ Use `delay()` or blocking operations
* ✗ Over-engineer for hypothetical future use cases
* ✗ Add external library dependencies

## CI/CD Pipeline

**GitHub Actions** (`.github/workflows/test.yml`) runs on push/PR to `master`:

1. **Lint Job**: Runs `make lint` - must pass for merge
2. **Test Job**:
   * Runs `make test` - unit tests
   * Generates coverage - uploads to Coveralls
   * Runs `make ci` - builds all examples for all platforms (~10 minutes)
   * Must pass for merge

**When CI Fails**:
* Lint errors: Check `make lint` output
* Test failures: Review test logs
* Build errors: Check platform-specific compilation issues

## Debugging Tips

**On Target Hardware:**
* Flash with `make upload`, monitor with `make monitor`
* Add `Serial.print()` statements for debugging
* Verify `Update()` is called regularly in `loop()` (every few milliseconds)
* Check pin numbers match your board's pinout

**Development:**
* Check HAL implementation if behavior differs between platforms
* For HAL issues: test simple on/off before complex effects
* Use unit tests to isolate core logic issues
* Check PlatformIO environment with `make envdump`

