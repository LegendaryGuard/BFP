@echo off
setlocal

set "SOURCEPK3FILE=.\*.pk3"
rem Change the path to save/overwrite the pk3 file:
set "BFPQ3DIR=%USERPROFILE%\Quake3\bfp"

if exist "%SOURCEPK3FILE%" (
    echo Moving the file...
    move "%SOURCEPK3FILE%" "%BFPQ3DIR%"
    TIMEOUT 3 > NUL
    echo COMPILED DATETIME:
    echo %DATE% %TIME:~0,8%
) else (
    echo ERROR! CHECK THE LOG
)

endlocal
