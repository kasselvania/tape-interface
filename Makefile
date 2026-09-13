.PHONY: build check verify clean

build:
	mkdir -p build
	./tape_sdk/tapp-build tapp/interface build/interface.tapp

check:
	$(MAKE) -C tape_sdk check

verify: build
	./tape_sdk/tools/verify-tapp.sh build/interface.tapp

clean:
	rm -f build/interface.tapp
