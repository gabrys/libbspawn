CC=gcc
CXX=g++
AR=ar
BOOST_VERSION=1.79.0
BOOST_DIRNAME=boost_1_79_0
BOOST_URL=https://boostorg.jfrog.io/artifactory/main/release/$(BOOST_VERSION)/source/$(BOOST_DIRNAME).tar.bz2

default: cspawn

clean:
	rm -rf build/
	rm -f cspawn

build/libbspawn.o: libbspawn.h libbspawn.cpp
	mkdir -p build/
	$(CXX) -Os -static -lstatic -Wno-narrowing -I build/$(BOOST_DIRNAME) -fPIC -c -o build/libbspawn.o libbspawn.cpp

build/libbspawn.a: build/libbspawn.o
	$(AR) rcs build/libbspawn.a build/libbspawn.o

cspawn: cspawn.c libbspawn.h build/libbspawn.a
	$(CC) -flto -Os --static -o cspawn cspawn.c build/libbspawn.a -lstdc++

get_boost:
	mkdir -p build
	cd build && wget -c $(BOOST_URL)
	cd build && rm -rf $(BOOST_DIRNAME)
	cd build && tar -xjf $(BOOST_DIRNAME).tar.bz2
