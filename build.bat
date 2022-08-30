set PATH=%~dp0\tools\bin;%PATH%

cd source\game
CALL game.bat
cd ..\..\

cd source\cgame
CALL cgame.bat
cd ..\..\

cd source\q3_ui
CALL q3_ui.bat
cd ..\..\

cd binaries
..\tools\bin\7za.exe a -tzip ..\pak9.pk3 vm
cd ..

rd binaries /s /q
rd intermediate /q /s
pause

:quit