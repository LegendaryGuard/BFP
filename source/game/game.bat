rem make sure we have a safe environement
set LIBRARY=
set INCLUDE=

mkdir ..\..\intermediate\vm\game
cd ..\..\intermediate\vm\game

set src=..\..\..\source
set cc=..\..\..\tools\bin\lcc.exe -DQ3_VM -S -Wf-target=bytecode -Wf-g -I%src%\cgame -I%src%\game -I%src%\q3_ui %1

%cc%  %src%/game/g_main.c
@if errorlevel 1 goto quit

%cc%  %src%/game/g_syscalls.c
@if errorlevel 1 goto quit

%cc%  %src%/game/bg_misc.c
@if errorlevel 1 goto quit
%cc%  %src%/game/bg_lib.c
@if errorlevel 1 goto quit
%cc%  %src%/game/bg_pmove.c
@if errorlevel 1 goto quit
%cc%  %src%/game/bg_slidemove.c
@if errorlevel 1 goto quit
%cc%  %src%/game/q_math.c
@if errorlevel 1 goto quit
%cc%  %src%/game/q_shared.c
@if errorlevel 1 goto quit

%cc%  %src%/game/ai_dmnet.c
@if errorlevel 1 goto quit
%cc%  %src%/game/ai_dmq3.c
@if errorlevel 1 goto quit
%cc%  %src%/game/ai_main.c
@if errorlevel 1 goto quit
%cc%  %src%/game/ai_chat.c
@if errorlevel 1 goto quit
%cc%  %src%/game/ai_cmd.c
@if errorlevel 1 goto quit
%cc%  %src%/game/ai_team.c
@if errorlevel 1 goto quit

%cc%  %src%/game/g_active.c
@if errorlevel 1 goto quit
%cc%  %src%/game/g_arenas.c
@if errorlevel 1 goto quit
%cc%  %src%/game/g_bot.c
@if errorlevel 1 goto quit
%cc%  %src%/game/g_client.c
@if errorlevel 1 goto quit
%cc%  %src%/game/g_cmds.c
@if errorlevel 1 goto quit
%cc%  %src%/game/g_combat.c
@if errorlevel 1 goto quit
%cc%  %src%/game/g_items.c
@if errorlevel 1 goto quit
%cc%  %src%/game/g_mem.c
@if errorlevel 1 goto quit
%cc%  %src%/game/g_misc.c
@if errorlevel 1 goto quit
%cc%  %src%/game/g_missile.c
@if errorlevel 1 goto quit
%cc%  %src%/game/g_mover.c
@if errorlevel 1 goto quit
%cc%  %src%/game/g_session.c
@if errorlevel 1 goto quit
%cc%  %src%/game/g_spawn.c
@if errorlevel 1 goto quit
%cc%  %src%/game/g_svcmds.c
@if errorlevel 1 goto quit
%cc%  %src%/game/g_target.c
@if errorlevel 1 goto quit
%cc%  %src%/game/g_team.c
@if errorlevel 1 goto quit
%cc%  %src%/game/g_trigger.c
@if errorlevel 1 goto quit
%cc%  %src%/game/g_utils.c
@if errorlevel 1 goto quit
%cc%  %src%/game/g_weapon.c
@if errorlevel 1 goto quit
%cc%  %src%/game/ai_vcmd.c
@if errorlevel 1 goto quit

rem set -vq3 flag for new q3asm, qvm's will run on original 1.32c binaries
..\..\..\tools\bin\q3asm.exe -vq3 -r -m -o qagame -f %src%/game/game
:quit
cd %src%/game
