@echo off
set mode=debug
if /i $%1 equ $release set mode=release
echo Running in %mode% mode.
if not exist build\%mode% exit /b 0
pushd build\%mode%
call vim log.txt
popd

echo Exit Code is %errorlevel%