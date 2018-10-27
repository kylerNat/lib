@echo off

REM mkdir "build"

del "tests/build/test1.exe"
clang -Wno-writable-strings -O2 -g -gcodeview -o "tests/build/test1.exe" -I "./" -D DEBUG "tests/test1.cpp" -lUser32.lib -lGdi32.lib -lopengl32.lib

if not errorlevel 1 (
   echo running...
   "tests/build/test1.exe"
) else (
   EXIT /B 1
)
rem popd
