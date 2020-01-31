all: configure build test

configure:
	node-gyp configure

build:
	node-gyp build

test: build
	cd test && node test.js

clean:
	node-gyp clean

.PHONY: configure build test