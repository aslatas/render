@echo off

set common_flags=/W3 /Gm- /EHsc /std:c++17 /nologo
set debug_flags=/Od /Z7
set release_flags=/O2 /GL /analyze- /D "NDEBUG"
set linker_flags=User32.lib Kernel32.lib Gdi32.lib /INCREMENTAL:no /NOLOGO
set include_dir=/I..\..\..\include /I..\..\..\src /I..\..\..\ext /I..\..\ext

set source_files=..\..\src\main.cpp


set mode=debug
if /i $%1 equ $release (set mode=release)
if %mode% equ debug (
set flags=%common_flags% %include_dir% %debug_flags% %source_files%
) else (
set flags=%common_flags% %include_dir% %release_flags% %source_files%
)
echo Building in %mode% mode.
echo.     -Cleaning output directory.
if not exist build\%mode% mkdir build\%mode%
pushd build\%mode%
if exist *.exe del /q *.exe
if exist *.pdb del /q *.pdb
if exist *.obj del /q *.obj
if exist *.dll del /q *.dll

:: BUILD
echo.     -Compiling:
call cl %flags% /link %linker_flags%
if %errorlevel% neq 0 (
echo Error during compilation!
popd
goto :fail
)
popd

echo Build complete!
exit /b 0

:fail
echo Build failed!
exit /b %errorlevel%