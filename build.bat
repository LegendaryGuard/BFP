set PATH=.\tools\bin;%PATH%

cd source\game
CALL game.bat
cd ..\..\

cd source\cgame
CALL cgame.bat
cd ..\..\

cd source\q3_ui
CALL q3_ui.bat
cd ..\..\

tools\bin\7za.exe a -tzip pak9.pk3 binaries\vm

rd binaries /s /q
rd intermediate /q /s

:quit