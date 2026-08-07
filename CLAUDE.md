# JLed: LED library for embedded devices

Non-blocking, time-driven C++14 library for LED control (blink, breathe, fade). LEDs group for parallel or sequential control.

**Hard constraints** (core logic in `src/`): no float, dynamic allocation, exceptions, RTTI, `delay()`, or blocking ops. Templates over virtual functions. Backwards-compatible public API; break only with a major version bump.

## Repository Structure

| Path             | Purpose                                          |
| ---------------- | ------------------------------------------------ |
| `src/`           | Library source (`.h`/`.cpp`)                     |
| `test/`          | Host unit tests (Catch2, separate Makefile)      |
| `examples/`      | MCU `.ino` sketches                              |
| `.tools/`        | Dev tools (doc site generator, act log analyser) |
| `platformio.ini` | PlatformIO config                                |
| `devbox.json`    | Dev environment                                  |

## Build & Test

PlatformIO + Make inside `devbox shell`. Discover targets with `make help`.
`test/Makefile` targets: `test`, `clean`, `clobber`, `coverage` (HTML in `test/report/`).

## Code Style

- **Format**: `.clang-format` (Google). `make format` / `make format-check` (legacy: `make lint`).
- **Static analysis**: `make lint-tidy` (clang-tidy); see `doc/LINTING_GUIDE.md`.
- **Naming**: `PascalCase` classes/methods, `snake_case_` private members, `kPascalCase` constants, `lowercase_t` aliases.
- Prefer `constexpr` over `#define`. No `constexpr` on functions with `if` (C++14 limit); `if (sizeof(Brightness) == 1)` stands in for `if constexpr` until C++17.
- Use `= delete` and `override`.

## Architecture

**Strict separation**: state machine / effect calculation / hardware access. Never put platform-specific code in `jled_base.h`.

- `src/jled_effects.h`/`.cpp`: `BrightnessEvaluator` + effects (`Blink`, `Breathe`, `Candle`, ...), `scale`/`lerp`/`fadeon_func`.
- `src/jled_base.h`: platform-agnostic `TJLed<Hal, Clock, B>` state machine + fluent API.
- `src/jled_group_base.h`: `TJLedGroup`, `TJLedAny`, `TJLedRef` (grouping, type erasure).
- `src/jled.h`: platform detection; exposes `JLed`, `JLedHD`, `JLedGroup`, `JLedAny`.
- `src/*_hal.h`: per-platform HAL (Arduino, ESP32, ESP8266, mbed, Pico, STM32Cube), two abstractions each: PWM (e.g. `ArduinoHal::analogWrite(Brightness)`) and Clock (e.g. `ArduinoClock::millis()`).

**Effects**: structs with `Period()` and `Eval(t)`, stateless and copyable (see `ConstantBrightnessEvaluator`).
**Resolution**: `JLed`/`JLedHD` are template instances; higher resolution = smoother PWM.
**Memory**: fixed buffers or placement new, never dynamic.

**Fluent API** via CRTP (methods return `B&`):

```cpp
JLed led = JLed(21).DelayBefore(1500).Breathe(500).Repeat(5).MaxBrightness(150);
```

## Testing

- Catch2 (amalgamated in `test/catch2/`). `TEST_CASE("what", "[tag]")`, `SECTION()` for variations. Tags: `[jled]`, `[sequence]`, `[hal]`. Test evaluators by calling `Eval(t)` at various time points. HAL mocks in `test/Arduino.h`, `test/esp-idf/`. Register new test files in `test/Makefile`.
- Tests are whitebox: read and understand the code path before writing a test for it, not just the public behavior.
- Keep tests simple: test the specific behavior at hand, don't add setup, helpers, or cases beyond what's needed to cover it.
- Use a fixture (`TEST_CASE_METHOD`) where applicable, e.g. `ArduinoMockFixture` in `test/test_arduino_hal.cpp`.

## Common Tasks

- **New effect**: evaluator in `src/jled_effects.h`, fluent method in `src/jled_base.h` (ref `BlinkBrightnessEvaluator` / `TJLed::Blink`). Add tests, an example, a `README.md` entry.
- **New HAL**: copy `src/arduino_hal.h`, add detection in `src/jled.h`, add `test/test_[platform]_hal.cpp`. The HAL concept's `analogWrite()` requires two arguments, `analogWrite<Color>(Color val, bool invert)`; if your HAL has no native inversion capability, implement a plain single-argument `analogWrite(Color val)` and wrap it in `InvertableHal<YourHal>` (`src/invertable_hal.h`) to get software inversion for free. The HAL concept also requires `void SetLowActive(bool)`, called once from `LowActive()`; a HAL with a hardware polarity register uses it to pre-arm that register, while `InvertableHal` implements it as a no-op for HALs without one. See `Esp32Hal`/`PicoHal` for examples of the former.
- **Bug fix**: failing test first, then fix, then `make test`, `make coverage`, `make lint`.

Every change adds tests. Run `make lint && make test` before commit. Don't change a test to make it pass; fix the code. Correctness over completeness: don't guess or invent APIs/files/configs; label assumptions; ask when unsure.

## CI/CD

GitHub Actions on push/PR to `master`: lint, then unit tests + coverage (Coveralls). All must pass.

`make ci-act` runs the build jobs locally via `act` (~10min): the single `examples` matrix, which covers the Arduino boards plus the dedicated `nucleo_f401re_mbed` and `nucleo_f401re_stm32cube` rows, logging NDJSON to `.act-logs/`:

```sh
.tools/act-log/act-log.py report          # summary table; exits 1 on failures
.tools/act-log/act-log.py report <unit>   # full log for one unit, e.g. uno or nucleo_f401re_mbed
```

A "unit" is one summary row: a matrix board (e.g. `uno`, `nucleo_f401re_mbed`). Status: `OK` built, `FAIL` build failed (code bug), `INFRA` never reached build (act issue, not code). Ignore NDJSON `jobResult` (buggy for parallel jobs); the analyser uses `stepResult`.

## Documentation Site

Auto-generated microsite at https://jandelgado.github.io/jled/ from git tags + master. Generator in `.tools/doc-site/` (see its `README.md`).
