.PHONY: build check verify test clean

HOST_CC ?= clang

build:
	mkdir -p build
	./tape_sdk/tapp-build tapp/interface build/interface.tapp
	./tape_sdk/tapp-build tapp/route_probe build/route_probe.tapp
	./tape_sdk/tapp-build tapp/route_source_probe build/route_source_probe.tapp

check:
	$(MAKE) -C tape_sdk check

verify: build
	./tape_sdk/tools/verify-tapp.sh build/interface.tapp
	python3 tests/check_imports.py build/interface.tapp
	./tape_sdk/tools/verify-tapp.sh build/route_probe.tapp
	./tape_sdk/tools/verify-tapp.sh build/route_source_probe.tapp
	python3 tests/check_probe_imports.py

test:
	mkdir -p build
	$(HOST_CC) -std=c11 -Wall -Wextra -Werror -U__ARM_FP -I tape_sdk tests/interface_native.c -o build/interface_native
	./build/interface_native
	$(HOST_CC) -std=c11 -Wall -Wextra -Werror -U__ARM_FP -I tape_sdk tests/route_probe_native.c -lm -o build/route_probe_native
	./build/route_probe_native

clean:
	rm -f build/interface.tapp build/interface_native
	rm -f build/route_probe.tapp build/route_source_probe.tapp build/route_probe_native
