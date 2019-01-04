@echo off

cl.exe 2>&1 | findstr /e /i x64 >nul: && goto :x64
cl.exe 2>&1 | findstr /e /i amd64 >nul: && goto :amd64
cl.exe 2>&1 | findstr /e /i x86 >nul: && goto :x86
echo ERROR: Failed to configure.
goto :eof

:x86
:amd64
echo AMD64=0 >config.mk
echo 386=1 >>config.mk
type config.mk
goto :eof

:x64
:amd64
echo AMD64=1 >config.mk
echo 386=0 >>config.mk
type config.mk
goto :eof
