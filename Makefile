all: build/main.o build/res.o
	gcc build/*.o -o Launcher.exe

build:
	mkdir build

build/res.o: build src/res.rc
	windres src/res.rc -o build/res.o

build/main.o: build src/main.c
	gcc -c src/main.c -o build/main.o

_PHONY: clean

clean:
	rm -rf build
	rm -f Launcher.exe
