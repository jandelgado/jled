# Linting & Formatting

JLed currently uses two, independent tool families side by side:

- `cpplint` — legacy, wired into CI (`make lint`). Keeps working unchanged.
- `clang-format` / `clang-tidy` — newer, opt-in local tools. Not yet wired
  into CI.

Neither replaces the other yet; this is intentional — `clang-tidy` is being
introduced gradually so any false positives can be tuned in `.clang-tidy`
before it becomes CI-enforced.

| Tool           | Role                        | Config           | Notes                                             |
| -------------- | ---------------------------- | ----------------- | -------------------------------------------------- |
| `clang-format` | formatting / style           | `.clang-format`   | single source of truth for `ColumnLimit` etc.      |
| `clang-tidy`   | static analysis / bug-finding | `.clang-tidy`     | does not format code; has no line-length check     |
| `cpplint`      | legacy style checker (CI)     | flags in Makefile | hardcodes its own `--linelength=100`               |

## Commands

- `make format` — auto-format `src`, `test`, `examples` in place
- `make format-check` — check formatting without modifying files (CI-friendly)
- `make lint-tidy` — run clang-tidy static analysis on `src`/`test`
- `make lint` — legacy cpplint check (unchanged, still what CI runs)

## Why there's no `-std=c++14 -Isrc -Itest` in the Makefile

The repo root already has a `compile_flags.txt` with the project's compile
flags. clang-tidy auto-discovers this by walking up from the file being
analyzed, so the flags live in exactly one place instead of being repeated
in the Makefile.

## Scope

- `format` / `format-check` cover `src` (flat), `test` (including the
  `esp-idf/`/`pico-sdk/` mock headers, excluding vendored `test/catch2` and
  build output `test/bin`/`test/report`), and `examples/<name>/*` (excluding
  the vendored, gitignored `examples/raspi_pico/pico-sdk/`).
- `lint-tidy` covers only the flat top level of `src`/`test`, matching what
  `compile_flags.txt` (`-Isrc -Itest`) can actually resolve.

## Known limitations

- `readability-identifier-naming` and `cppcoreguidelines-*` are not enabled
  yet in `.clang-tidy` — see the comments in that file for why and for a
  starting config if enabling them later.
- `src/pico_hal.h`, `test/test_esp32_hal.cpp`, and `test/test_mbed_hal.cpp`
  may report errors under `make lint-tidy`: the real test build adds
  per-target flags (`-I./pico-sdk`, `-DESP32`, `-D__MBED__`) that the
  generic root `compile_flags.txt` doesn't carry. Pre-existing gap, not
  something this change fixes.
- Not wired into CI yet — local, opt-in only.
