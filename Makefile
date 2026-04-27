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

# Seed ~/.cache/act/ before the parallel run. Parallel matrix jobs race on
# git-cloning the same actions simultaneously, corrupting the download
# (https://github.com/nektos/act/issues/1943, closed won't-fix).
# Run one real job first to fully populate the action cache; --action-offline-mode
# on the main run then prevents any re-cloning during the parallel execution.
ACT_CONTAINER_OPTS = -v $(CURDIR)/.pio-cache:/home/runner/.platformio

ci-act: phony
	@set -e; \
	OUTDIR=$$(mktemp -d "$(CURDIR)/.act-run.XXXXXX"); \
	echo "Act output dir: $$OUTDIR"; \
	mkdir -p "$(CURDIR)/.pio-cache"; \
	# Seed: run one real job so ~/.cache/act/ is populated before the parallel run. \
	$(RUN) act --job examples \
	    --matrix board:uno --matrix example:hello --env ACT=true \
	    --container-options "$(ACT_CONTAINER_OPTS)" \
	    || true; \
	ACT_LOG=$$OUTDIR/act.ndjson; \
	$(RUN) act --job examples --json --action-offline-mode \
	    --env ACT=true \
	    --container-options "$(ACT_CONTAINER_OPTS)" \
	    2>&1 | tee $$ACT_LOG | jq -Rr 'try (fromjson | .msg // empty) catch empty' || true; \
	jq -Rr 'try (fromjson | select(.matrix.board? and .matrix.example?) | "\(.matrix.board)\t\(.matrix.example)\t\(tojson)") catch empty' $$ACT_LOG \
	    | while IFS=$$'\t' read -r board example json; do \
	        printf '%s\n' "$$json" >> $$OUTDIR/$${board}_$${example}.ndjson; \
	    done; \
	printf '=== Summary ===\n'; \
	jq -Rr 'try (fromjson | select(.jobResult != null and .matrix.board != null) | [if .jobResult == "success" then "OK" else "FAIL" end, .matrix.board, .matrix.example] | @tsv) catch empty' $$ACT_LOG \
	    | sort | grep . | column -t


envdump: phony
	-$(RUN) pio run --target envdump

clean: phony
	-$(RUN) pio run --target clean
	make -C test clean
	rm -f src/{*.o,*.gcno,*.gcda}
	rm -rf .doc-site/

upload: phony
	$(RUN) pio run --target upload

monitor: phony
	$(RUN) pio device monitor

docs: phony
	$(RUN) .tools/doc-site/generate_site.py --output .doc-site

tags: phony
	$(RUN) ctags -R
