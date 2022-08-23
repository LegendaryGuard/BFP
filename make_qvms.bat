set PATH=.\tools\bin;%PATH%


CALL make_qvm_game.bat
CALL make_qvm_cgame.bat
CALL make_qvm_ui.bat
CALL make_pak9.bat

:quit