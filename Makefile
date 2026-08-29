# use this makefile to build with platformio
#
SHELL := /bin/bash
.PHONY: phony
RUN := $(if $(shell which pio 2>/dev/null),,$(if $(shell which devbox 2>/dev/null),devbox run --,))

help: phony ## show available make targets (this help)
	@grep -E '^[a-zA-Z_%-]+:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "  %-12s %s\n", $$1, $$2}' | sort

pio-config: phony ## dump platformio.ini configuration information
	@$(RUN) pio project config

list-examples: phony ## list of available examples
	@find ./examples  -maxdepth 1 -type d -printf "%f\n"|sort

build: phony ## builds example (according to platformio.ini)
	$(RUN) pio run

build-%: phony ## builds a specific example for a given plaftorm. e.g., `make build-candle ENV=esp32`
	$(RUN) PLATFORMIO_SRC_DIR=examples/$* pio run -e ${ENV}

upload: phony ## flash firmware to connected device (according to platformio.ini)
	$(RUN) pio run --target upload

upload-%: phony ## build and upload specific example for a given plaftorm. e.g., `make upload-candle ENV=esp32`
	$(RUN) PLATFORMIO_SRC_DIR=examples/$* pio run --target upload -e ${ENV}

envdump: phony ## dump PlatformIO environment configuration
	$(RUN) pio run --target envdump

monitor: phony ## open serial monitor for connected device
	$(RUN) pio device monitor

lint: phony ## run the C++ linter
	$(RUN) cpplint --filter -readability/check,-build/include_subdir \
		    --quiet\
		    --linelength=100\
		    --exclude test/catch2 \
		    --extensions=cpp,h,ino $(shell find . -maxdepth 3 \( ! -regex '.*/\..*' \) \
		       -type f -a \( -name "*\.cpp" -o -name "*\.h" -o -name "*\.ino" \) )

format: phony ## auto-format src/test/examples in-place with clang-format
	$(RUN) find src -maxdepth 1 -type f \( -name "*.cpp" -o -name "*.h" \) -exec clang-format -i {} +
	$(RUN) find test -type f \( -name "*.cpp" -o -name "*.h" \) \
		    ! -path "test/catch2/*" ! -path "test/bin/*" ! -path "test/report/*" \
		    -exec clang-format -i {} +
	$(RUN) find examples -maxdepth 2 -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.ino" \) -exec clang-format -i {} +

format-check: phony ## check formatting without modifying files (clang-format --dry-run)
	$(RUN) find src -maxdepth 1 -type f \( -name "*.cpp" -o -name "*.h" \) -exec clang-format --dry-run --Werror {} +
	$(RUN) find test -type f \( -name "*.cpp" -o -name "*.h" \) \
		    ! -path "test/catch2/*" ! -path "test/bin/*" ! -path "test/report/*" \
		    -exec clang-format --dry-run --Werror {} +
	$(RUN) find examples -maxdepth 2 -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.ino" \) -exec clang-format --dry-run --Werror {} +

lint-tidy: phony ## run clang-tidy static analysis on src/test (modern, complements 'lint')
	$(RUN) find src -type f \( -name "*.cpp" -o -name "*.h" \) -exec clang-tidy {} \;
	$(RUN) find examples -maxdepth 2 -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.ino" \) -exec clang-tidy {} \;

lint-tidy-tests: phony ## run clang-tidy static analysis on src/test (modern, complements 'lint')
	$(RUN) find test -maxdepth 1 -type f \( -name "*.cpp" -o -name "*.h" \) -exec clang-tidy {} \;

test: phony ## run unit tests with coverage, plus doc-site generator tests
	$(RUN) .tools/doc-site/test_generate_site.py
	$(RUN) $(MAKE) -j "$(shell getconf _NPROCESSORS_ONLN)" -C test coverage

ACT_CACHE_DIR      = $(HOME)/.cache/act/jled-cache
ACT_CACHE          = --cache-server-path $(ACT_CACHE_DIR)
ACT_CONTAINER_OPTS = --user $(shell id -u):$(shell id -g)
# jobs run by ci-act: the single examples matrix (Arduino boards plus the
# dedicated nucleo_f401re_mbed and nucleo_f401re_stm32cube rows)
ACT_JOBS           = examples

ci-act: phony ## run full build matrix locally via act (runs ~10min)
	@rm -rf "$(CURDIR)/.act-logs" && mkdir -p "$(CURDIR)/.act-logs"
	@for job in $(ACT_JOBS); do \
	    $(RUN) act --job $$job --json --action-offline-mode \
	        -W "$(CURDIR)/.github/workflows/test.yml" \
	        $(ACT_CACHE) \
	        --container-options "$(ACT_CONTAINER_OPTS)" \
	        2>&1 | tee -a "$(CURDIR)/.act-logs/act.ndjson" \
	        || true; \
	done
	$(RUN) .tools/act-log/act-log.py report

clean: phony ## remove build artifacts and generated files
	-$(RUN) pio run --target clean
	make -C test clean
	rm -f src/{*.o,*.gcno,*.gcda}
	rm -rf .doc-site/
	rm -rf .act-logs/
	rm -f tags

docs: phony ## generate documentation site to .doc-site/
	$(RUN) .tools/doc-site/generate_site.py --output .doc-site

run-docs-server: phony ## serve the generated doc site on http://localhost:8000
	python3 -m http.server 8000 --directory .doc-site

tags: phony ## generate ctags for src/ and test/
	$(RUN) ctags -R --exclude='examples/raspi_pico/pico-sdk/*' --exclude='*json' --exclude='test/report/*' --exclude='test/catch2/*' src/ test/
