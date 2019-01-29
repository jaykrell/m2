# FIXME use cmake

# NOTE: This one Makefile works with Microsoft nmake and GNU make.
# They use different conditional syntax, but each can be nested and inverted within the other.

all: check

ifdef MAKEDIR:
!ifdef MAKEDIR

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

ifdef MAKEDIR:
!ifdef MAKEDIR

!if !exist (./config.mk)
!if [.\config.cmd]
!endif
!endif
!if exist (./config.mk)
!include ./config.mk
!endif

#!message AMD64=$(AMD64)
#!message 386=$(386)

!if !defined(AMD64) && !defined(386)
AMD64=1
386=0
!endif

!if $(AMD64)
win=winamd64.exe
386=0
!elseif $(386)
win=winx86.exe
AMD64=0
!endif

!ifndef win
win=win.exe
!endif

all: $(win)

config:
	.\config.cmd

check:

run: $(win)
	.\$(win) $(WINDIR)\Microsoft.NET\Framework\v2.0.50727\mscorlib.dll

debug: $(win)
!if $(AMD64)
	\bin\amd64\cdb .\$(win) $(WINDIR)\Microsoft.NET\Framework\v2.0.50727\mscorlib.dll
!elseif $(386)
	\bin\x86\cdb .\$(win) $(WINDIR)\Microsoft.NET\Framework\v2.0.50727\mscorlib.dll
!endif

clean:
	$(RM_F) $(win) m2.obj *.ilk

# TODO clang cross
#
#mac: m2.cpp
#	g++ -std=c++17 -g m2.cpp -o $@
#

# TODO /Qspectre

$(win): m2.cpp
	@-del $(@R).pdb $(@R).ilk
	cl $(Wall) /W4 /MD /Zi /EHsc $** /link /out:$@ /incremental:no

!else
else

UNAME_S = $(shell uname -s)

ifeq ($(UNAME_S), Cygwin)
Cygwin=1
NativeTarget=cyg
else
Cygwin=0
Linux=0
endif

ifeq ($(UNAME_S), Linux)
Linux=1
NativeTarget=lin
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
	$(RM_F) mac win32 win32.exe win64 win64.exe win win.exe cyg cyg.exe *.ilk lin

mac: m2.cpp
	g++ -std=c++17 -g m2.cpp -o $@

cyg: m2.cpp
	g++ -std=c++17 -g m2.cpp -o $@

lin: m2.cpp
	g++ -std=c++17  -Wall -g m2.cpp -o $@

win32.exe: m2.cpp
	i686-w64-mingw32-g++ -std=c++17 -g m2.cpp -o $@

win64.exe: m2.cpp
	x86_64-w64-mingw32-g++ -std=c++17 -g m2.cpp -o $@

endif
!endif :
