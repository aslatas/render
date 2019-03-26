@echo off
set mode=debug
if /i $%1 equ $release set mode=release
echo Running in %mode% mode.
if not exist build\%mode% exit /b 0
pushd build\%mode%
call Win32PlatformLayer.exe > log.txt 2> err.txt
popd

echo Exit Code is %errorlevel%