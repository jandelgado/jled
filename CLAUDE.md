# JLed LED library for embedded devices

JLed is an embedded C++ library for non-blocking, time-driven LED control (blink, breathe, fade, etc.). LEDs can be grouped and controlled in parallel or sequentially.

- **Language**: C++14 (both `src/` and `test/`), embedded-friendly: no exceptions, RTTI, or dynamic allocation
- **Build**: PlatformIO + Make, managed via `devbox shell`
- **Key constraint**: Non-blocking, time-driven; never use `delay()`

## Repository Structure

| Path             | Purpose                                           |
| ---------------- | ------------------------------------------------- |
| `src/`           | Library source (`.h`/`.cpp`)                      |
| `test/`          | Host-based unit tests (Catch2, separate Makefile) |
| `examples/`      | MCU `.ino` sketches                               |
| `.tools/`        | Dev tools (doc site generator)                    |
| `platformio.ini` | PlatformIO config                                 |
| `devbox.json`    | Dev environment (Python 3.13, lcov, cpplint, pio) |

## Build & Test

- `Makefile` targets: `lint`, `test`, `ci` (build for all platforms examples ~10min), `envdump`
- `test/Makefile` targets: `test`, `clean`, `clobber`, `coverage` (HTML report in `test/report/`)

## Code Style

- **Formatting**: `.clang-format` (Google style); run `make lint`
- **Naming**: `PascalCase` classes/methods, `snake_case_` private members, `kPascalCase` constants, `lowercase_t` type aliases
- Prefer `constexpr` over `#define`; no `constexpr` on functions with `if` (C++14 limit)
- Use `= delete`, `override`; no float in core logic; templates over virtual functions

## Architecture

**Strict separation**: state machine logic / effect calculation / hardware access.

**Core files:**

- `src/jled_base.h` — platform-agnostic: `TJLed<Hal, Clock, B>`, `BrightnessEvaluator`, all effects, `TJLedSequence`
- `src/jled.h` — platform detection (preprocessor macros), exposes `JLed`, `JLedHD`, `JLedSequence`
- `src/*_hal.h` — HAL per platform (Arduino, ESP32, ESP8266, mbed, Pico)

**HAL**: each platform has two abstractions in `src/*_hal.h`:

- **PWM HAL** (e.g. `ArduinoHal`) — `analogWrite(Brightness val)`
- **Clock** (e.g. `ArduinoClock`) — `static uint32_t millis()`

**Effects**: simple structs with `Period()` and `Eval(t)`. Must be stateless and copyable (see
ConstantBrightnessEvaluator)

**Resolution**: `JLed`/`JLedHD` etc. are template instances; higher resolution = smoother PWM.

**Memory**: no dynamic memory allocation, use fixed buffers or placement new.

**Fluent API** via CRTP — methods return `B&`:

```cpp
JLed led = JLed(21).DelayBefore(1500).Breathe(500).Repeat(5).MaxBrightness(150);
```

**Technical debt (C++14 compat)**: `if (sizeof(Brightness) == 1)` instead of `if constexpr` — acceptable until C++17 is baseline in Arduino ecosystem.

## Testing

- Framework: Catch2 (amalgamated in `test/catch2/`)
- Naming: `TEST_CASE("what is tested", "[tag]")`, use `SECTION()` for variations
- Tags: `[jled]`, `[sequence]`, `[hal]`
- Test effect evaluators by calling `Eval(t)` at various time points
- HAL mocks: see `test/Arduino.h`, `test/esp-idf/`
- Add new test files to `test/Makefile`

## Common Tasks

**New effect** (`src/jled_base.h`):

1. use `BlinkBrightnessEvaluator` and `TJLed::Blink` etc. as reference
2. Add tests in `test/test_jled.cpp`, example in `examples/`, update `README.md`

**New HAL**: use `src/arduino_hal.h` as reference → create `src/[platform]_hal.h` → add detection in `src/jled.h` → add tests in `test/test_[platform]_hal.cpp`

**Bug fix**: write failing test first → fix → `make test` → `make coverage` → `make lint`

## Do's and Don'ts

**DO**: preserve API backwards compatibility · add tests for all changes · `make lint && make test` before commit

**DON'T**:

- Add platform-specific code to `jled_base.h`
- Use float in core logic
- Use dynamic allocation, exceptions, or RTTI
- Use `delay()` or blocking operations
- Break public API without a major version bump
- **Change a test to make it pass** — fix the code instead

## CI/CD

GitHub Actions (`.github/workflows/test.yml`) on push/PR to `master`: lint → unit tests + coverage (Coveralls) → `make ci`. All must pass.

## Documentation Site

Auto-generated microsite at https://jandelgado.github.io/jled/ from git tags + master. Generator in `.tools/doc-site/` (see its `README.md`).
