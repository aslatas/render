@echo off
::      Set build tool and library paths as well as compile flags here.       ::
::----------------------------------------------------------------------------::
set debug_flags=/Od /Z7 /MT
set release_flags=/O2 /GL /MT /analyze- /D "NDEBUG"
set common_flags=/W3 /Gm- /EHsc /std:c++17 /nologo /Fe: NYCE.exe /I ..\..\include /I ..\..\ext ..\..\src\Win32PlatformLayer.cpp
set linker_flags=User32.lib Kernel32.lib Gdi32.lib /INCREMENTAL:no /NOLOGO

::        Run the build tools, but only if they aren't set up already.        ::
::----------------------------------------------------------------------------::
cl >nul 2>nul
if %errorlevel% neq 9009 goto :build
echo Running VS build tool setup.
echo Initializing MS build tools...
call windows\setup_cl.bat
cl >nul 2>nul
if %errorlevel% neq 9009 goto :build
echo Unable to find build tools! Make sure that you have Microsoft Visual Studio 10 or above installed!
exit /b 1

::      Set the build mode, and create the build directory if necessary.      ::
::----------------------------------------------------------------------------::
:build
set mode=debug
if /i $%1 equ $release (set mode=release)
if %mode% equ debug (
set flags=%common_flags% %debug_flags%
) else (
set flags=%common_flags% %release_flags%
)
echo Building in %mode% mode.
if not exist build\%mode% mkdir build\%mode%
pushd build\%mode%

::                          Perform the actual build.                         ::
::----------------------------------------------------------------------------::
echo.     -Compiling:
call cl %flags% /link %linker_flags%
del *.obj
if %errorlevel% neq 0 (
echo Error during compilation!
popd
goto :fail
)
popd

::      Compile shaders and stuff. Needs replaced with a better system.       ::
::----------------------------------------------------------------------------::
echo.     -Compiling shaders:
pushd shaders
call compile.cmd
if %errorlevel% neq 0 (
echo Error during shader compilation!
popd
goto :fail
)

popd

::     Copy resources. Should be replaced by a proper hotloading system.      ::
::----------------------------------------------------------------------------::
if not exist build\%mode%\resources mkdir build\%mode%\resources
if not exist build\%mode%\resources\shaders mkdir build\%mode%\resources\shaders
robocopy shaders\spv build\%mode%\resources\shaders *.spv /move /mir /ns /nc /nfl /ndl /np /njh /njs

echo.     -Copying Textures:
if not exist build\%mode%\resources\textures mkdir build\%mode%\resources\textures
robocopy resources\textures build\%mode%\resources\textures /mir /ns /nc /ndl /np /njh /njs

echo.     -Copying Fonts:
if not exist build\%mode%\resources\fonts mkdir build\%mode%\resources\fonts
robocopy resources\fonts build\%mode%\resources\fonts /mir /ns /nc /ndl /np /njh /njs

echo.     -Copying Models:
if not exist build\%mode%\resources\models mkdir build\%mode%\resources\models
robocopy resources\models build\%mode%\resources\models /mir /ns /nc /ndl /np /njh /njs

echo Build complete!
exit /b 0

:fail
echo Build failed!
exit /b %errorlevel%