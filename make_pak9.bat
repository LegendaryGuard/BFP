set PATH=C:\quake3\tools\bin;%PATH%

REM cd binaries
tools\bin\7za.exe a -tzip pak9.pk3 binaries\vm
move pak9.pk3 binaries

:quit