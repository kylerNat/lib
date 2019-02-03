@echo off

REM mkdir "build"

del "tests/build/test1.exe"
del "tests/build/webgen_test.exe"
clang -Wno-writable-strings -O2 -g -gcodeview -o "tests/build/test1.exe" -I "./" -D DEBUG "tests/test1.cpp" -lUser32.lib -lGdi32.lib -lopengl32.lib
rem clang -Wno-writable-strings -O2 -g -o "tests/build/webgen_test.exe" -I "./" -D DEBUG "tests/webgen_test.cpp"

if not errorlevel 1 (
   echo running...
   "tests/build/test1.exe"
   rem "tests/build/webgen_test.exe"
) else (
   EXIT /B 1
)
rem popd
