AR=ar
CC=gcc
CXX=g++

lib: build/libbspawn.a

clean:
	rm -f build/lib* cspawn cspawn.exe

build/libbspawn.o: libbspawn.h libbspawn.cpp
	mkdir -p build/
	$(CXX) -Os -static -lstatic -Wno-narrowing -fPIC -c -o build/libbspawn.o libbspawn.cpp

build/libbspawn.a: build/libbspawn.o
	$(AR) rcs build/libbspawn.a build/libbspawn.o

# test C program linking statically to our library for Linux
cspawn: cspawn.c libbspawn.h build/libbspawn.a
	$(CC) -flto=auto -Os -static -pthread -o cspawn cspawn.c build/libbspawn.a \
		-lboost_filesystem -lboost_atomic -lstdc++

# ... and for Windows
cspawn.exe: cspawn.c libbspawn.h build/libbspawn.a
	$(CC) -flto=auto -Os -static -pthread -o cspawn.exe cspawn.c build/libbspawn.a \
		-lboost_filesystem -lboost_atomic -lstdc++ -lbcrypt -lws2_32
