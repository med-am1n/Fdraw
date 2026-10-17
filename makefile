.PHONY: build run clean

build:
	cmake -B build
	cmake --build build

run:
	cmake -B build
	cmake --build build
	./build/app

clean:
	rm -rf build

