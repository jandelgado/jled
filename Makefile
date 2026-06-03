# use this makefile to build with platformio
#
SHELL := /bin/bash
.PHONY: phony
RUN := $(if $(shell which devbox 2>/dev/null),devbox run --,)

all: phony
	$(RUN) pio run

lint: phony
	$(RUN) cpplint --filter -readability/check,-build/include_subdir \
		    --linelength=100\
		    --exclude test/catch2 \
		    --extensions=cpp,h,ino $(shell find . -maxdepth 3 \( ! -regex '.*/\..*' \) \
		       -type f -a \( -name "*\.cpp" -o -name "*\.h" -o -name "*\.ino" \) )

test: phony
	$(RUN) $(MAKE) -C test coverage

# Parallel matrix jobs race on git-cloning the same actions simultaneously
# (https://github.com/nektos/act/issues/1943, closed won't-fix).
# Strategy: run warmup jobs one board at a time (no parallel races), restore
# the .gitignore files act removes between each run, then run examples with
# --action-offline-mode and with the warmup dependency stripped so there is no
# warmup stage 0 racing in parallel.
LOCAL_WORKFLOW    := $(CURDIR)/.act-logs/jled-test-local.yml
ACT_CACHE_DIR      = $(HOME)/.cache/act/jled-cache
ACT_CACHE          = --cache-server-path $(ACT_CACHE_DIR)
ACT_CONTAINER_OPTS = --user $(shell id -u):$(shell id -g)

ci-act: phony
	@rm -rf "$(CURDIR)/.act-logs" && mkdir -p "$(CURDIR)/.act-logs"; \
	# Seed uno online: populates ~/.cache/act/ action cache for offline mode. \
	$(RUN) act --job warmup \
	    --matrix board:uno \
	    $(ACT_CACHE) \
	    --container-options "$(ACT_CONTAINER_OPTS)" \
	    || true; \
	# Warm up each remaining board sequentially (offline) to fill act cache \
	# without triggering parallel .gitignore races. \
	for board in esp01 nano33ble esp32dev rpipico nucleo_f401re; do \
	    $(RUN) act --job warmup \
	        --matrix board:$$board \
	        --action-offline-mode \
	        $(ACT_CACHE) \
	        --container-options "$(ACT_CONTAINER_OPTS)" \
	        || true; \
	done; \
	# Run examples only (warmup done above; strip needs:[warmup] so there is no \
	# parallel warmup stage 0 that would race on the same .gitignore files). \
	sed '/^    needs: \[warmup\]$$/d' "$(CURDIR)/.github/workflows/test.yml" \
	    > "$(LOCAL_WORKFLOW)"; \
	$(RUN) act --job examples --json --action-offline-mode \
	    -W "$(LOCAL_WORKFLOW)" \
	    $(ACT_CACHE) \
	    --container-options "$(ACT_CONTAINER_OPTS)" \
	    2>&1 | tee "$(CURDIR)/.act-logs/act.ndjson" \
	    | jq -Rr 'try (fromjson | select(.raw_output != true and (.msg | test("(✅|❌)"))) | "[" + (.matrix.board // "?") + "/" + (.matrix.example // "?") + "] " + (.msg | ltrimstr("  "))) catch empty' \
	    || true; \
	$(RUN) .tools/act-log/act-log.py report


envdump: phony
	-$(RUN) pio run --target envdump

clean: phony
	-$(RUN) pio run --target clean
	make -C test clean
	rm -f src/{*.o,*.gcno,*.gcda}
	rm -rf .doc-site/
	rm -rf .act-logs/

upload: phony
	$(RUN) pio run --target upload

monitor: phony
	$(RUN) pio device monitor

docs: phony
	$(RUN) .tools/doc-site/generate_site.py --output .doc-site

tags: phony
	$(RUN) ctags -R
