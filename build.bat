set PATH=%~dp0\tools\bin;%PATH%
set q3path=C:\Games\quake3\Xx

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
..\tools\bin\7za.exe a -tzip %q3path%\bfpq3\pak0.pk3 vm
cd ..

rd binaries /s /q
rd intermediate /q /s

%q3path%\quake3e.x64 +set fs_game bfpq3 +devmap testmap

:quit