all:
	rm -rf ./bin
	mkdir -p ./bin
	clang -std=c99 ./src/main.c -o ./bin/limelight -O2 -F/System/Library/PrivateFrameworks -framework Cocoa -framework Carbon -framework SkyLight -Wall -pedantic

install:
	install ./bin/limelight /usr/local/bin