# FIXME use cmake

# FIXME winarm64 etc.
all: mac win32 win64

run: mac
	./mac /s/mono/sdks/out/bcl/net_4_x/mscorlib.dll

clean:
	-rm mac win 2>/dev/null

mac: m2.cpp
	g++ m2.cpp -o $@

win32: m2.cpp
	i686-w64-mingw32-g++ m2.cpp -o $@

win64: m2.cpp
	x86_64-w64-mingw32-g++ m2.cpp -o $@
