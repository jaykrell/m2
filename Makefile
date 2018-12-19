# FIXME use cmake

# FIXME winarm64 etc.
all: mac win32.exe win64.exe

run: mac
	./mac /s/mono/mcs/class/lib/build-macos/mscorlib.dll

clean:
	rm -f mac win32 win32.exe win64 win64.exe win win.exe

mac: m2.cpp
	g++ -std=c++17 -g m2.cpp -o $@

win32.exe: m2.cpp
	i686-w64-mingw32-g++ -std=c++17 -g m2.cpp -o $@

win64.exe: m2.cpp
	x86_64-w64-mingw32-g++ -std=c++17 -g m2.cpp -o $@
