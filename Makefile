# use this makefile to build with platformio
#
SHELL := /bin/bash
.PHONY: phony
RUN := $(if $(shell which devbox 2>/dev/null),devbox run --,)

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
		    --linelength=100\
		    --exclude test/catch2 \
		    --extensions=cpp,h,ino $(shell find . -maxdepth 3 \( ! -regex '.*/\..*' \) \
		       -type f -a \( -name "*\.cpp" -o -name "*\.h" -o -name "*\.ino" \) )

test: phony ## run unit tests with coverage
	$(RUN) $(MAKE) -C test coverage

ACT_CACHE_DIR      = $(HOME)/.cache/act/jled-cache
ACT_CACHE          = --cache-server-path $(ACT_CACHE_DIR)
ACT_CONTAINER_OPTS = --user $(shell id -u):$(shell id -g)

ci-act: phony ## run full build matrix locally via act (runs ~10min)
	@rm -rf "$(CURDIR)/.act-logs" && mkdir -p "$(CURDIR)/.act-logs"
	$(RUN) act --job examples --json --action-offline-mode \
	    -W "$(CURDIR)/.github/workflows/test.yml" \
	    $(ACT_CACHE) \
	    --container-options "$(ACT_CONTAINER_OPTS)" \
	    2>&1 | tee "$(CURDIR)/.act-logs/act.ndjson" \
	    || true
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
