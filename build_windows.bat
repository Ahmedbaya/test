@echo off
REM Windows build script for MOKP DLL
REM Requires MinGW or similar GCC-compatible compiler

echo Building MOKP DLL for Windows...

REM Clean previous builds
if exist *.o del *.o
if exist *.dll del *.dll
if exist *.exe del *.exe
if exist *.a del *.a

REM Compile object files
echo Compiling source files...
gcc -Wall -Wextra -O2 -fPIC -DMOKP_DLL_EXPORTS -c mokp_dll.c -o mokp_dll.o
if errorlevel 1 goto error

gcc -Wall -Wextra -O2 -fPIC -c mokp_core.c -o mokp_core.o
if errorlevel 1 goto error

gcc -Wall -Wextra -O2 -fPIC -c indicators.c -o indicators.o
if errorlevel 1 goto error

REM Build DLL
echo Building DLL...
gcc -shared -o mokp.dll mokp_dll.o mokp_core.o indicators.o -lm -Wl,--out-implib,libmokp.a
if errorlevel 1 goto error

REM Build test program
echo Building test program...
gcc -Wall -Wextra -O2 -c test_mokp_dll.c -o test_mokp_dll.o
if errorlevel 1 goto error

gcc -o test_mokp_dll.exe test_mokp_dll.o mokp_dll.o mokp_core.o indicators.o -lm
if errorlevel 1 goto error

echo.
echo Build completed successfully!
echo Files created:
echo   - mokp.dll (DLL library)
echo   - libmokp.a (import library)
echo   - test_mokp_dll.exe (test program)
echo.
echo To test the DLL, run: test_mokp_dll.exe
goto end

:error
echo.
echo Build failed with errors!
exit /b 1

:end