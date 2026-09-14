.PHONY: build check verify test clean

HOST_CC ?= clang

build:
	mkdir -p build
	./tape_sdk/tapp-build tapp/interface build/interface.tapp

check:
	$(MAKE) -C tape_sdk check

verify: build
	./tape_sdk/tools/verify-tapp.sh build/interface.tapp
	python3 tests/check_imports.py build/interface.tapp

test:
	mkdir -p build
	$(HOST_CC) -std=c11 -Wall -Wextra -Werror -U__ARM_FP -I tape_sdk tests/interface_native.c -o build/interface_native
	./build/interface_native

clean:
	rm -f build/interface.tapp build/interface_native
