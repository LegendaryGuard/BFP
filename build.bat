@echo off
set PATH=%~dp0\tools\bin;%PATH%

set ERROR_CGAME_QVM=0
set ERROR_QAGAME_QVM=0
set ERROR_UI_QVM=0

rem Change some number to this variable if you want to look binaries and intermediate directories to check .asm files, 
rem also .map files in binaries\vm\ to look the list of mapped addresses, functions and cvars.
rem ------
rem Cyrax: external jts (jump targets segment) was created in opposition to ioq3' VM_MAGIC_VER2 which is not 
rem compatible with Q3 SDK license
rem you need that data(jump targets segment) to avoid buggy bytecode generation from engine side
rem so there were 2 choices for mod developers: 
rem 1) generate V2 binaries and release mod sources (because it isn't compatible with original engine / Q3 SDK)
rem 2) keep old V1 format and prepare for bugs on 1.32c or slowdown on ioq3
rem with q3e you have 3rd variant: use my q3asm and generate 1.32c-compatible qvms + external jts-file than will 
rem help with bytecode generation
rem ------
rem If NO_JTS=1 is set, .jts files won't be available if the operation compiled successfully
set NO_JTS=0
rem If NO_MAP=1 is set, these directories and .map files won't be available if the operation compiled successfully
set NO_MAP=1

rem Remove binaries, intermediate and pak9.pk3 if these are already here to clean up
if exist %~dp0\binaries\ (
  echo    The directory called binaries exists, removing...
  rd binaries /s /q
)

if exist %~dp0\intermediate\ (
 echo    The directory called intermediate exists, removing...
 rd intermediate /q /s
)

if exist %~dp0\pak9.pk3 (
  echo    pak9.pk3 exists, removing...
  del pak9.pk3
)

@echo on
cd source\game
CALL game.bat
cd ..\..\

cd source\cgame
CALL cgame.bat
cd ..\..\

cd source\q3_ui
CALL q3_ui.bat
cd ..\..\
             
@echo off
echo.
echo.
cd binaries

if exist vm\cgame.jts ( if %NO_JTS% NEQ 0 ( del vm\cgame.jts ) )
if exist vm\cgame.map ( if %NO_MAP% NEQ 0 ( del vm\cgame.map ) )
if exist vm\qagame.jts ( if %NO_JTS% NEQ 0 ( del vm\qagame.jts ) )
if exist vm\qagame.map ( if %NO_MAP% NEQ 0 ( del vm\qagame.map ) )
if exist vm\ui.jts ( if %NO_JTS% NEQ 0 ( del vm\ui.jts ) )
if exist vm\ui.map ( if %NO_MAP% NEQ 0 ( del vm\ui.map ) )

echo ***************************************
rem -- CGAME --
if not exist vm\cgame.qvm ( 
  set ERRORLEVEL=1 
  set ERROR_CGAME_QVM=1 
) else ( echo              vm\cgame.qvm )
if exist vm\cgame.jts ( echo              vm\cgame.jts )
if exist vm\cgame.map ( echo              vm\cgame.map )

rem -- QAGAME --
if not exist vm\qagame.qvm ( 
  set ERRORLEVEL=1 
  set ERROR_QAGAME_QVM=1 
) else ( echo              vm\qagame.qvm )
if exist vm\qagame.jts ( echo              vm\qagame.jts )
if exist vm\qagame.map ( echo              vm\qagame.map )

rem -- UI --
if not exist vm\ui.qvm ( 
  set ERRORLEVEL=1 
  set ERROR_UI_QVM=1 
) else ( echo              vm\ui.qvm )
if exist vm\ui.jts ( echo              vm\ui.jts )
if exist vm\ui.map ( echo              vm\ui.map )
echo ***************************************

if %ERRORLEVEL% NEQ 0 (
  echo.
  if %ERROR_CGAME_QVM% NEQ 0 ( echo    ERROR: cgame.qvm COULD NOT BE COMPILED )
  if %ERROR_QAGAME_QVM% NEQ 0 ( echo    ERROR: qagame.qvm COULD NOT BE COMPILED )
  if %ERROR_UI_QVM% NEQ 0 ( echo    ERROR: ui.qvm COULD NOT BE COMPILED )
  echo.
  goto :quit
)

@echo on
..\tools\bin\7za.exe a -tzip ..\pak9.pk3 vm
cd ..

@echo off
if %NO_MAP% NEQ 0 (
echo.
echo    Removing binaries and intermediate directories...
rd binaries /s /q
rd intermediate /q /s
echo.
) else (
  echo.
  echo    You can look binaries and intermediate directories
  echo.
)

:quit
set ERROR_CGAME_QVM=
set ERROR_QAGAME_QVM=
set ERROR_UI_QVM=
set NO_JTS=
set NO_MAP=