all: configure build test

configure:
	node-gyp configure

build:
	node-gyp build

test: build
	cd test && npm test

clean:
	node-gyp clean

.PHONY: configure build test