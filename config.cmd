@echo off

cl.exe 2>&1 | findstr /e /i x64 >nul: && goto :x64
cl.exe 2>&1 | findstr /e /i amd64 >nul: && goto :amd64
echo ERROR: Failed to configure.
goto :eof

:x64
:amd64
echo AMD64=1 >config.mk
echo 386=0 >>config.mk
