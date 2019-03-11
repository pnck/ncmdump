release:
	mkdir -p build_release
	cd build_release && cmake -DCMAKE_BUILD_TYPE=RELEASE .. && make
	mv -f build_release/ncmdump .
debug:
	mkdir -p build_debug
	cd build_debug && cmake -DCMAKE_BUILD_TYPE=DEBUG -DCMAKE_VERBOSE_MAKEFILE=1 .. && make
	mv -f build_debug/ncmdump .
all: release

install: all
	mv ncmdump /usr/local/bin

clean:
	rm -rf build_release build_debug
