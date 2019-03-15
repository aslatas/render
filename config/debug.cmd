@echo off
if $%1==$rebuild (
    echo Rebuilding:
    call build.cmd
    if %errorlevel% neq 0 (exit /b %errorlevel%)
)
if not exist build\debug (
    echo Unable to find build directory! Try building in debug mode first, or call debug with argument <rebuild>.
    exit /b 0
)
pushd build\debug
call devenv ParseConfig.exe
popd