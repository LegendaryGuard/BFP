Bid For Power (BFP) source code
===============================
[![Stars](https://img.shields.io/github/stars/LegendaryGuard/BFP)](https://github.com/LegendaryGuard/BFP/stargazers)
[![Forks](https://img.shields.io/github/forks/LegendaryGuard/BFP)](https://github.com/LegendaryGuard/BFP/forks)
[![License](https://img.shields.io/github/license/LegendaryGuard/BFP)](#legal)
[![Build actions](https://img.shields.io/github/actions/workflow/status/LegendaryGuard/BFP/build.yml)](https://github.com/LegendaryGuard/BFP/actions)
![Visits](https://badges.pufler.dev/visits/LegendaryGuard/BFP)


A legendary Quake 3 Arena mod from the late 90s to early 2000s.

<p align="center">
	<img src="https://github-production-user-asset-6210df.s3.amazonaws.com/49716252/267147041-b5a8fb8c-575b-4b48-b6fe-513000717559.png" alt="bfpq3logo" width=350 />
</p>

# TODO list

### Pending:

- [ ] Attacksets (configurable for cfgs)
- [ ] Cvars as described on old docs
- [ ] Power Struggles (when two beam attacks collide)
- [ ] Skin Config File (explosionModel, explosionShader, missileRotation, missileShader, … look old docs and cfgs about that: Custom plugin models)
- [ ] 21 different ki attacks including controllable, homing, and chargeable attacks (no guns) (can be referenced to some previous tasks)
- [ ] 6 different selectable characters, each with 5 attacks (can be referenced to some previous tasks)

### Done:

- [x] ~~Animations as listed on the old docs~~
- [x] ~~Auras~~
- [x] ~~Balanced player physics movements (different from the original BFP, but balanced for gameplay)~~
- [x] ~~Bind key to recover ki energy~~
- [x] ~~Bind key to toggle speed (ki boost). HINT: HASTE POWERUP~~
- [x] ~~Blocking (consumes ki energy, transfers all damage to ki instead of health, deflect missile attacks, more info on old docs)~~
- [x] ~~Breakable map entities ("func_breakable")~~
- [x] ~~Gametype: Survival (`g_gametype 3`)~~
- [x] ~~Gametype: Monster (`g_gametype 4`)~~
- [x] ~~Gametype: Team Last Man Standing (`g_gametype 6`)~~
- [x] ~~Hit Stun (makes player can't use ki, melee, block and charge)~~
- [x] ~~Instant character model changing~~
- [x] ~~Ki trails (use ki to move, cg_kiTrail >10 )~~
- [x] ~~Melee feature~~
- [x] ~~Remove some items like powerups and health pickups~~
- [x] ~~Powerlevel and Power Tiers indicated on old docs~~
- [x] ~~Playable third person mode and first person vis mode~~
- [x] ~~Remove weapon visuals (models and stuff)~~
- [x] ~~Replace ammo to ki energy stamina~~
- [x] ~~Short-Range Teleport - Zanzoken (when pressing 2 times left or right)~~
- [x] ~~Third person traceable crosshair~~
- [x] ~~Toggeable flight (bind key). Hint: FLIGHT POWERUP~~
- [x] ~~Transformations (related to Power Tiers)~~


## Table of contents
> 1. [History](#history)
> 2. [About the repository](#about-the-repository)
> 3. [References and clues to know how should be the game](#references-and-clues-to-know-how-should-be-the-game)
> 4. [How to build](#how-to-build)
> > 4.1. [Introduction](#introduction)<br/>
> > 4.2. [Windows](#windows)
> > > 4.2.1. [Building QVM (using .bat)](#building-qvm-using-bat)<br/>
> > > 4.2.2. [Building QVM (mingw)](#building-qvm-mingw)<br/>
> > > 4.2.3. [MSYS2 (mingw) (Building dynamic libraries (.dll))](#msys2-mingw-building-dynamic-libraries-dll)<br/>
> > > 4.2.4. [Cygwin (mingw) (Building dynamic libraries (.dll))](#cygwin-mingw-building-dynamic-libraries-dll)
> >
> > 4.3. [Linux](#linux)
> > > 4.3.1. [Building QVM (make)](#building-qvm-make)<br/>
> > > 4.3.2. [Building QVM (using .bat)](#building-qvm-using-bat-1)<br/>
> > > 4.3.3. [Building shared libraries (.so)](#building-shared-libraries-so)
> >
> > 4.4. [Optional](#optional)
> 5. [Notes](#notes)
> 6. [Legal](#legal)
> 7. [Credits](#credits)

# History

![BFP_ZEQ2_history](https://github-production-user-asset-6210df.s3.amazonaws.com/49716252/267147557-7954d397-3df4-4cf7-b9c3-d62e393658ab.png)

Started: [1998](https://goldenhammersoftware.blogspot.com/2010/07/big-mountain-snowboarding-history-and.html) <br/> 
Ended: 2002

Bid For Power is a total conversion for QuakeIII that plays nothing like the original game. Players take control of Ki-powered superheros and battle it out in a mostly aerial fight. The game is highlighted by the work of a great art team and an original style, and the gameplay is extremely fast paced. It can be difficult to keep up with until you get the hang of it.

The project was started on Quake 2, that's where they were getting organized playing Quake and learn some programming there.

The source code is said to have started from SDK 1.17 point release after the release of Quake 3 Arena (December 2, 1999). It continued to be updated, including the addition of `cg_particles.c` in 1.29, until 1.31. 
More info can be found in [Quake 3 Arena changelog version history](https://discourse.ioquake.org/t/quake-3-changelog-version-history/375).

The original source code appears to be lost, but the assets and some docs are available in various places. Nonetheless, not all sources are accessible.

### Old dev journals (1998 - 2002)

The original URLs can be slower to load, so it's recommended to use markdown edition ones.

NOTE: Some URLs and images within these contents may be broken or partially recovered. The final parts of Chris and Yrgol dev journals were found here:<br/> 
Chris: https://web.archive.org/web/20011202065630/http://bidforpower.com/ <br/>
Yrgol: https://web.archive.org/web/20020520060044/http://www.planetquake.com/Bidforpower/ <br/>
On markdown editions, the data log is complete.

- Chris dev journal: https://web.archive.org/web/20020210145200/http://bidforpower.com/journals/chris.php

    * [Chris dev journal (Markdown edition)](docs/old_dev_journals/chris_dev_journal.md)

- Yrgol dev journal: https://web.archive.org/web/20020205150340/http://www.bidforpower.com/journals/yrgol.php

    * [Yrgol dev journal (Markdown edition)](docs/old_dev_journals/yrgol_dev_journal.md)

- Ansel dev journal: https://web.archive.org/web/20011203063814/http://bidforpower.com/journals/ansel.php

    * [Ansel dev journal (Markdown edition)](docs/old_dev_journals/ansel_dev_journal.md)

- Anthony dev journal: https://web.archive.org/web/20020210151755/http://bidforpower.com/journals/anthony.php

    * [Anthony dev journal (Markdown edition)](docs/old_dev_journals/anthony_dev_journal.md)

- Dash dev journal: https://web.archive.org/web/20020223210411/http://www.bidforpower.com/journals/dash.php

    * [Dash dev journal (Markdown edition)](docs/old_dev_journals/dash_dev_journal.md)

- Rodney Olmos dev journal: https://web.archive.org/web/20011218204129/http://bidforpower.com/journals/rodney.php

    * [Rodney Olmos dev journal (Markdown edition)](docs/old_dev_journals/rodney_dev_journal.md)

- PyroFragger dev journal: https://web.archive.org/web/20011218203246/http://bidforpower.com/journals/pyrofragger.php

    * [PyroFragger dev journal (Markdown edition)](docs/old_dev_journals/pyrofragger_dev_journal.md)

- Remisser dev journal: https://web.archive.org/web/20020210152114/http://bidforpower.com/journals/remisser.php

    * [Remisser dev journal (Markdown edition)](docs/old_dev_journals/remisser_dev_journal.md)

... And more dev journals can be found [here](https://web.archive.org/web/20011202135731/http://bidforpower.com/journals/).

# About the repository

We're making a replica of the lost source code. <br/>
The highest priority goal is to copy and recreate the complete logical structure of the BFP game. It would be a game SDK that'll provide a structured and standardized way to modify the mod.<br/>
You'll notice some differences and things that the original Bid For Power didn't have/were forgotten, incomplete or poorly made such as:
- some adjusted UI buttons
- BFP OPTIONS menu is upgraded, big explosions and smoke options are back (these were removed after RC/beta versions), also shell and ring options are available and these are options are interactive with explosion type option. Sprite and particle aura types are available on aura type option
- SERVER INFO menu displays all server info (on original BFP, the info was badly displayed and nothing was shown) and it has pagination
- DRIVER INFO menu is fixed (on original BFP crashes) and it has pagination on extensions
- explosion dynamic lights are back (these were broken after RC/beta versions)
- how particles move (e.g. bubbles are handled underwater and touching something solid vanishes to save performance)
- `g_allowSpectatorChat` cvar is functional, spectators can't send messages if the cvar is disabled
- spectator mode can toggle ki boost as if the player is flying
- particle aura is almost implemented
- while charging ki near water, bubble particles appear
- when player is still moving with/without friction and charging ki, antigrav rock particles appear
- new particles: charge smoke, which appears when charging ki near the ground
- removed some unused cvars
- balanced player pmove physics: 
  - players can interact bounce pads like Q3 does (on original BFP, the physics are like you're sliding heavily on the ground and not bouncing as usually do, you're being pushed when touching bounce pads)
  - no weird underwater movements while going intentionally down and moving crazily fast touching the ground (this might be a bug/glitch from original BFP)
  - no specified stuck animation, so any animation is correctly handled when being stuck (on original BFP, when the player is stuck or pretty near to something solid, the reason is still unknown though. It does a jumping forward/backward animation, that doesn't make sense)
  - water movement handling is different from original BFP, but it works similarly
- survival gamemode is pretty well balanced (on original BFP, when everytime the player changes a different character model from their own preffix, dies and respawns during warmup, the warmup resets. So, that's unfair)
- monster/oozaru gamemode has the following in-game differences compared to the original:
  - the player monster is labeled 'MON' on the scoreboard so teammates can identify them quickly
  - the player monster has a larger floating sprite chat
  - the player monster has a larger shadow effect similar to regular players
  - the player monster has a large dynamic light when charging or using ki
  - the player monster's ki trails are larger than other players'
  - the player monster generates bigger bubbles, smoke, and antigrav rock particles when charging/using ki boost
  - player monster's first-person and first-person vis mode viewpoints work properly
  - player monster's third-person camera has improved focus (similar to BFP's standard third-person view but scaled for giant characters)
  - `g_monster` cvar enables the monster/big monkey feature (that happened in the RC/beta versions)
  - available in the UI, maps marked as `monster` besides `oozaru` can be viewed in the UI
- team last man standing has the following in-game differences compared to the original:
  - players cannot switch teams after joining a team (on original BFP, the dead player -who was forced to spectate- can join during the match, that was against the rules)
  - players cannot voluntarily switch to spectator after joining a team
  - players attempting to switch teams/spectate receive centerprint messages
- some cvars didn't save changed values after quitting the game that happened on original BFP (means that `CVAR_ARCHIVE` wasn't on them), but these are now applied on replica
- file size differences between the QVM and the original BFP QVM can be quite significant
- kiCharge, boostCost and blockCost cvars work differently
- ...

Any fixes, improvements and contributions are welcome. But we can't accept secondary things and other stuff that don't reach the goals.

# References and clues to know how should be the game

Documentations, references and extracted stuff will give us clues to reach the goals. <br/>

- Old documentations:

    * [Guide](docs/Guide.md)
    * [Creating custom plugin models](docs/Create_Custom_Models.md)

<br/>

- Cvars, cmd and bind stuff about the old game:

    * [Bindlist](docs/bind_bfp_list.txt)
    * [Cmdlist](docs/cmd_bfp_list.txt)
    * [Cvarlist](docs/cvar_bfp_list.txt)

<br/>

- **cfg files**:

A sample inside `models/players/player_name/default.cfg`:

   * [default.cfg](cfgs/default.cfg)

Server config:

   * [bfp_server.cfg](cfgs/bfp_server.cfg)

Attacksets:

   * [bfp_attacksets.cfg](cfgs/bfp_attacksets.cfg)

Weapon settings:

   * [bfp_weapon.cfg](cfgs/bfp_weapon.cfg)
   * [bfp_weapon2.cfg](cfgs/bfp_weapon2.cfg)

BFP config (optional - general binding and some client stuff, unused. WARNING: when executing, game might crash):

   * [bfp.cfg](cfgs/bfp.cfg)


# How to build

### Introduction

*IMPORTANT NOTE TO THE DEVELOPMENT*: all source code files must be **UTF-8 without BOM** and **Unix (LF)**, otherwise, it will cause compiler errors when using MakefileQVM (most likely, syntax error will be displayed).

- `.map` file is a linker map file, which is generated by the linker when it links together multiple object files into an executable or shared library. It contains information about the symbols (such as functions and variables) defined in each object file, as well as their addresses in the final executable or library.
The information in the map file can be useful for debugging and performance analysis. For example, it can help you identify which functions are taking the most time to execute, or which functions are being called from which parts of the code.

- The external JTS (Jump Target Segment) (`.jts`) files are used to improve the bytecode generation in the Quake 3 engine. The JTS file contains information about jump targets that are used by the VM (Virtual Machine) to execute the bytecode. It helps to avoid bugs, improve performance and it can be beneficial for better stability. JTS files are obtained compiling from q3asm, which can generate 1.32c-compatible QVMs along with the external JTS file.

- ### Windows:

    * #### _Building QVM (using .bat)_: 

    1. Keep in mind you must be in the repository directory. Execute `build.bat` to compile qvms.

    2. Once compiled successfully, look for `pak9.pk3`, copy and paste into `baseq3/` or mod Q3 game directory.

    Alternatively, in `build.bat`, you can set `NO_MAP=0` to obtain .map files, you can see them in binaries and intermediate directories.

    You can set `NO_JTS=1`, if you don't want to obtain .jts files inside pk3 file.

    * #### _Building QVM (mingw)_: 

    Note: that also uses 7z tool to compress them in a pk3 file.

    1. If you're using MSYS2 and you didn't install the prerequisites, follow the steps (from step 1 to step 4) on [MSYS2 (mingw)](#msys2-mingw-building-dynamic-libraries-dll) section.

    If you're using Cygwin and you didn't install the prerequisites, follow the steps (from step 1 to step 2) on [Cygwin (mingw)](#cygwin-mingw-building-dynamic-libraries-dll) section.

    2. Keep in mind you must be in the repository directory. To compile qvms, execute:
    ```sh
    make -f MakefileQVM
    ```

    3. Once compiled successfully, look for `pak9.pk3`, copy and paste into `baseq3/` or mod Q3 game directory. You can look `vm/` where you can see the objects and compiled files.

    Alternatively, you can execute:
    ```sh
    make -f MakefileQVM NO_JTS=1
    ```
    If you don't want to obtain .jts files inside pk3 file.

    Clean the compiled objects with:
    ```sh
    make -f MakefileQVM clean
    ```

    * #### _MSYS2 (mingw) (Building dynamic libraries (.dll))_:

    IMPORTANT NOTE: Not tested on Windows 32-bit. MSYS2 comes with multilib disabled in gcc (means you can't compile for x86 in a 64-bit system), more info [here](https://sourceforge.net/p/msys2/discussion/general/thread/3941f2c9/).


    To build, follow these instructions:

    1. Install msys2 from https://msys2.github.io/, following the instructions there.It doesn’t matter which version you download, just get one appropriate for your OS.

    2. Start "MSYS2 MinGW 64-bit" from the Start Menu. If you're using 32-bit system, use "MSYS2 MinGW 32-bit".

    3. Install mingw-w64-x86_64-gcc:
    ```sh
    pacman -S mingw-w64-x86_64-gcc
    ```
    32-bit:
    ```sh
    pacman -S mingw-w64-i686-gcc
    ```
    4. Install make:
    ```sh
    pacman -S make
    ```

    5. Go to the directory where you cloned the repository and compile with make
    ```sh
    make ARCH=x86_64
    ```
    32-bit:
    ```sh
    make ARCH=x86 WINDRES="windres -F pe-i386"
    ```

    6. Find the dlls in `build/release-mingw64-x86_64`, for 32-bit: `build/release-mingw32-x86`. <br/>
    
    If you can't compile 32-bit builds with MSYS2 MinGW, try [Cygwin](#cygwin-mingw-building-dynamic-libraries-dll) section.<br/>

    * #### _Cygwin (mingw) (Building dynamic libraries (.dll))_: 

    Detailed guide based on a [post by MAN-AT-ARMS](https://discourse.ioquake.org/t/how-to-build-ioquake3-using-cygwin/223).

    1. Install Cygwin

    Download the Cygwin setup package from http://cygwin.com/install.html.

    Choose either the 32-bit or 64-bit environment. 32-bit will work fine on both 32 and 64 bit versions of Windows. The setup program is also your Cygwin environment updater. If you have an existing Cygwin environment, the setup program will, by default, update your existing packages.

    Choose where you want to install Cygwin. The entire environment is self-contained in it's own folder, but you can also interact with files from outside the environment if you want to as well. The default install path is `C:\Cygwin`.
    Choose a mirror to download packages from, such as the [kernel.org](https://kernel.org/) mirrors.
    Choose a "storage area" for your package downloads.

    2. Package selection

    The next screen you see will be the package selections screen. In the upper left is a search box. This is where you will want to search for the necessary packages.

    These are the package names you'll want to search for:

    1- `mingw64-i686-gcc-core` (For building 32bit binaries)<br/>
    2- `mingw64-i686-gcc-g++` (Also for 32bit... C++ support... not required for the game, but useful for compiling other software)<br/>
    3- `mingw64-x86_64-gcc-core` (For building 64bit binaries)<br/>
    4- `mingw64-x86_64-gcc-g++` (For 64bit, same as above)<br/>
    5- `make`<br/>
    6- `bison`<br/>
    7- `git`

    3. Open Cygwin, go to the directory where you cloned the repository and compile with `make`

- ### Linux:

    * #### _Building QVM (make)_: 

    Note: that uses 7z tool to compress them in a pk3 file. If you don't have 7z tool, install with:
    ```sh
    sudo apt-get install p7zip-full
    ```

    1. If you didn't install the prerequisites, follow the step 1 on [Building shared libraries (.so)](#building-shared-libraries-so).

    2. Keep in mind you must be in the repository directory. To compile qvms, execute:
    ```sh
    make -f MakefileQVM
    ```

    3. Once compiled successfully, look for `pak9.pk3`, copy and paste into `baseq3/` or mod Q3 game directory. You can look `vm/` where you can see the objects and compiled files.

    Alternatively, you can execute:
    ```sh
    make -f MakefileQVM NO_JTS=1
    ```
    If you don't want to obtain .jts files inside pk3 file.
    
    Clean the compiled objects with:
    ```sh
    make -f MakefileQVM clean
    ```

    Optionally, you can set the destination directory, so the pk3 file will appear on this directory:
    ```sh
    make -f MakefileQVM DESTDIR=/your/path/q3/baseq3mod
    ```

    * #### _Building QVM (using .bat)_: 

    1. The alternative to execute and get the compiled qvms with `build.bat` requires [`wine` package](https://www.winehq.org/). So, in that part, needs the i386 package:
    ```sh
    sudo dpkg --add-architecture i386 && sudo apt-get update && sudo apt-get install wine32-development
    ```
    But it could be executed without using 32-bit package, if your system supports 64-bits. Go to WineHQ page anyways.
    
    2. Keep in mind, you must be in the repository directory to execute the script:
    ```sh
    wine cmd /c build.bat
    ```

    3. Once compiled successfully, look for `pak9.pk3`, copy and paste into `baseq3/` or mod Q3 game directory.

    * #### _Building shared libraries (.so)_:
    
    1. If you don't have gcc tools, install the build-essential packages, which is also known as a meta-package, it contains the GCC compiler all the other essentials used to compile the software written in C and C++ language.
    Also, requires `libc6-dev-i386` for x86 builds and `g++-multilib` and `gcc-mingw-w64` for cross-compiling. More info about MinGW question in Linux [here](https://stackoverflow.com/questions/44389963/how-to-install-mingw32-on-ubuntu).
    ```sh
    sudo apt-get install build-essential libc6-dev-i386 g++-multilib gcc-mingw-w64
    ```

    2. Simply execute (`-j4` is the number of parallel jobs you want to run during the compilation, in that case is set to 4): 
    ```sh
    make -j4
    ```
    3. And find .so files in `build/release-linux-x86_64`, for 32-bit: `build/release-linux-x86`. <br/><br/>


- ### Optional:

    You can use the optional part, if you followed and used some of these sections: 
    - [MSYS2 (mingw) (Building dynamic libraries (.dll))](#msys2-mingw-building-dynamic-libraries-dll)
    - [Cygwin (mingw) (Building dynamic libraries (.dll))](#cygwin-mingw-building-dynamic-libraries-dll)
    - [Building shared libraries (.so)](#building-shared-libraries-so)
    
    You can execute optionally the parameters using the following ways:

    * To compile debug x86 .so builds:
    ```sh
    make debug ARCH=x86 PLATFORM=linux # compiles debug x86 .so builds (creates "debug-linux-x86" directory inside "build")
    ```

    * To compile release x86 .dll builds:
    ```sh
    make ARCH=x86 PLATFORM=windows # compiles release x86 .dll builds (creates "release-windows-x86" directory inside "build")
    ```

    ... Optionally, you can play the parameters like `ARCH=x86_64` (compiles 64-bits builds), `PLATFORM=windows` (compiles dlls), `PLATFORM=linux` (compiles shared libraries (.so files)) ...

    * To compile and copy release builds at the destination directory, `DESTDIR` parameter is mandatory:
    ```sh
    make install DESTDIR=/your/path/q3/baseq3mod # compiles release builds and copy the builds to the destination directory (you can also put ARCH=x86 PLATFORM=windows if you want)
    ```

    * To compile and copy debug builds at the destination directory, `DESTDIR` parameter is mandatory:
    ```sh
    make install DESTDIR=/your/path/q3/baseq3mod # compiles debug builds and copy the builds to the destination directory (you can also put ARCH=x86 PLATFORM=windows if you want)
    ```

<br/>

# Notes

**IMPORTANT NOTE**: This repository was initialized from https://github.com/marconett/q3a.

#### Added source code files:

- cg_cvar.h
- cg_trails.c
- g_cvar.h
- ui_bfpoptions.c
- ui_cvar.h
- ui_mem.c
- ui_mp3decoder.c
- ui_mp3decoder.h

#### Removed source code files from the build tools:

- ui_cinematics.c
- ui_mods.c
- ui_playersettings.c
- ui_splevel.c
- ui_sppostgame.c
- ui_spskill.c

#### Unused source code files and unavailable in the build tools:

- ui_rankings.c
- ui_rankstatus.c
- ui_signup.c
- ui_specifyleague.c
- ui_spreset.c

# Legal

The mod source code is [GPLv3 licensed](./COPYING), the source code contents are based on Quake III Arena which is [GPLv2 licensed](./GPL-2).

The ancient abandoned MP3 decoder (`ui_mem.c`, `ui_mem.h`, `ui_mp3decoder.c` and `ui_mp3decoder.h`) is based on code from various contributors:
- Copyright (C) 1993 Sun Microsystems
- Copyright (C) 1995-1997 Michael Hipp
- Copyright (C) 1999 Aaron Holtzman
- Copyright (C) 2000-2001 Tim Angus

The images, screenshots, and URLs, even in the docs, included in this repository are used for reference purposes only and are not covered by the GPL license. They're sourced from various locations and are subject to their respective copyrights and terms of use.

### Bid For Power name

Nobody owns the "Bid For Power" name. Bid For Power was founded by Chris James and likely ended up in the palm of Yrgol's hand (although Yrgol doesn't own the assets). The owner, the maintainer and the contributors of the repository don't own this name. <br/>
This does not give any single person or a group of people to sell the name, basically it belongs to the original community. <br/>
The Bid For Power team may provide sufficient security against any claims or improper use of the name.

### Disclaimer

The game elements such as characters, and events depicted in this game are fictitious. Any resemblance to actual persons, living or deceased, or real events is purely coincidental.

# Credits

Bid For Power is made by these staff members. We don't own materials such as art designs, maps and character models from their assets.

<div align="center">

### Bid For Power Staff Members

<h4>
Ansel<br/>
Skin Artist<br/><br/>

Anthony<br/>
2D Artist<br/>

Chris<br/>
Founder<br/>

Dash<br/>
Level Designer / Texture Artist<br/>

Disco Stu<br/>
Web Designer<br/>

Gangsta Poodle<br/>
Level Designer<br/>

Kit Carson<br/>
Level Designer / Texture Artist<br/>

NilreMK<br/>
Modeler / Animator<br/>

Number17<br/>
Sound Engineer<br/>

Pyrofragger<br/>
Modeler / Animator<br/>

Remisser<br/>
Sound / Music Engineer<br/>

Rodney<br/>
Modeler / Animator<br/>

Yngwie<br/>
Level Designer / Texture Artist<br/>

Yrgol<br/>
Project Lead, Lead Programmer<br/>

::Additional Assistance::<br/>
Mooky, Perfect Chaos, Dakota, Bardock, DethAyngel, Ebola, Badhead, $onik, Gigatron, Timex & Nat.
</h4>
</div>
