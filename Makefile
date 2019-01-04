# FIXME use cmake

# NOTE: This one Makefile works with Microsoft nmake and GNU make.
# They use different conditional syntax, but each can be nested and inverted within the other.

all: check

ifdef _NMAKE_VER:
!ifdef _NMAKE_VER

#
# Microsoft nmake on Windows with desktop CLR, Visual C++.
#

RM_F = del 2>nul /f
#ILDASM = ildasm /nobar /out:$@
#ILASM = ilasm /quiet
#RUN_EACH=for %%a in (
#RUN_EACH_END=) do @$Q$(MONO)$Q %%a

!else
else

#
# GNU/Posix make on Unix with mono, gcc, clang, etc.
#
RM_F = rm -f

#ILDASM = ikdasm >$@
#ILASM = ilasm
#MONO ?= mono
#RUN_EACH=for a in
#RUN_EACH_END=;do $(MONO) $${a}; done

endif
!endif :

check:

clean:

exe:

ifdef _NMAKE_VER:
!ifdef _NMAKE_VER

!if !exist (./config.mk)
!if [.\config.cmd]
!endif
!endif
!include config.mk

#!message AMD64=$(AMD64)
#!message 386=$(386)

!if $(AMD64)
win=winamd64.exe
!endif

!if $(386)
win=winx86.exe
!endif

all: $(win)

config:
	.\config.cmd

check:

#run: $(win)
#	./mac /s/mono/mcs/class/lib/build-macos/mscorlib.dll
#
#debug: $(win)
#	lldb --  ./mac /s/mono/mcs/class/lib/build-macos/mscorlib.dll

clean:
	$(RM_F) $(win) m2.obj *.ilk

# TODO clang cross
#
#mac: m2.cpp
#	g++ -std=c++17 -g m2.cpp -o $@
#

$(win): m2.cpp
	cl /MD /Zi /EHsc /std:c++14 $** /link /out:$@ /incremental:no

!else
else

UNAME_O = $(shell uname -o)

ifeq ($(UNAME_O), Cygwin)
Cygwin=1
NativeTarget=cyg
else
Cygwin=0
endif

# TODO Darwin, Linux, etc.

# FIXME winarm64 etc.
all: $(NativeTarget) win32.exe win64.exe

run: $(NativeTarget)
	./$(NativeTarget) /s/mono/mcs/class/lib/build-macos/mscorlib.dll

debug: mac
	lldb -- ./$(NativeTarget) /s/mono/mcs/class/lib/build-macos/mscorlib.dll

clean:
	$(RM_F) mac win32 win32.exe win64 win64.exe win win.exe cyg cyg.exe *.ilk

mac: m2.cpp
	g++ -g m2.cpp -o $@

cyg: m2.cpp
	g++ -g m2.cpp -o $@

win32.exe: m2.cpp
	i686-w64-mingw32-g++ -g m2.cpp -o $@

win64.exe: m2.cpp
	x86_64-w64-mingw32-g++ -g m2.cpp -o $@

endif
!endif :
