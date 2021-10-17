@echo off
set CURR_DIR=%cd%

if not defined DevEnvDir (
    call "%VS_CMD_LINE_BUILD_PATH%" x64
)

cd /d %CURR_DIR%

REM The above doesn't always do the trick - this does the same thing
"C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64

SET headers=/I"%CURR_DIR%"
SET opts= -Zi /MP4 /W3 /EHsc /Fedumpling.exe /D_CRT_SECURE_NO_WARNINGS /FC /std:c++latest
SET sources=..\*.c
SET linkeropts=/SUBSYSTEM:Console opengl32.lib gdi32.lib dsound.lib

pushd bin
cl %opts% %headers% %sources% /link %linkeropts%
popd
