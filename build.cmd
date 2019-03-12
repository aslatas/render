@echo off
::      Set build tool and library paths as well as compile flags here.       ::
:: ---------------------------------------------------------------------------::
set toolpath=C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build
set common_flags=/W3 /Gm- /EHsc /std:c++17 /nologo
set debug_flags=/Od /Z7
set release_flags=/O2 /GL /analyze- /D "NDEBUG"
set linker_flags=User32.lib Kernel32.lib Gdi32.lib /INCREMENTAL:no /NOLOGO
set include_dir=/I..\..\include /I..\..\ext

set source_files=..\..\src\Main.cpp ..\..\src\Win32PlatformLayer.cpp ..\..\src\RenderBase.cpp ..\..\src\VulkanFunctions.cpp ..\..\src\RenderTypes.cpp ..\..\src\VulkanLoader.cpp ..\..\src\ModelLoader.cpp ..\..\src\Font.cpp ..\..\src\Texture.cpp ..\..\src\VulkanInit.cpp

::        Run the build tools, but only if they aren't set up already.        ::
:: ---------------------------------------------------------------------------::
cl >nul 2>nul
if %errorlevel% neq 9009 goto :build
echo Running VS build tool setup.
if exist "%toolpath%" (
pushd "%toolpath%"
if exist vcvarsall.bat (
echo Initializing MS build tools...
call vcvarsall.bat x64 > nul
popd
goto :build
)
) 
echo Unable to find build tools! Make sure that you have Microsoft Visual Studio 2017 Community Edition installed!
exit /b 1

::    Build in the correct mode, copying resources and whatnot as needed.     ::
:: ---------------------------------------------------------------------------::
:build
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

::      Compile shaders and stuff. Needs replaced with a better system.       ::
:: ---------------------------------------------------------------------------::
echo.     -Compiling shaders:
pushd shaders
call compile.cmd
if %errorlevel% neq 0 (
echo Error during shader compilation!
popd
goto :fail
)

popd
if not exist build\%mode%\shaders mkdir build\%mode%\shaders
robocopy shaders build\%mode%\shaders *.spv /move /mir /ns /nc /nfl /ndl /np /njh /njs

echo.     -Copying Textures:
if not exist build\%mode%\textures mkdir build\%mode%\textures
robocopy textures build\%mode%\textures /mir /ns /nc /ndl /np /njh /njs

echo.     -Copying Fonts:
if not exist build\%mode%\fonts mkdir build\%mode%\fonts
robocopy fonts build\%mode%\fonts /mir /ns /nc /ndl /np /njh /njs

echo.     -Copying Models:
if not exist build\%mode%\resources mkdir build\%mode%\resources
if not exist build\%mode%\resources\models mkdir build\%mode%\resources\models
robocopy resources\models build\%mode%\resources\models /mir /ns /nc /ndl /np /njh /njs

echo Build complete!
exit /b 0

:fail
echo Build failed!
exit /b %errorlevel%