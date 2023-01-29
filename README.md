Bid For Power (BFP) source code
===============================

A legendary 90s era Quake 3 Arena mod.

<p style="text-align: center;">
<img src="https://user-images.githubusercontent.com/49716252/186026745-458849b2-385b-4108-8881-e55e82fb1493.png" alt="zeqlogo" width=320 />
</p>


## **WARNING!** UNDER CONSTRUCTION!

### Table of contents
> > 1. [TODO list](#todo-list)
> > 2. [History](#history)
> > 3. [About the repository](#about-the-repository)
> > 4. [References and clues to know how should be the game](#references-and-clues-to-know-how-should-be-the-game)
> > 5. [How to build](#how-to-build)
> > > 5.1. [Windows](#windows)
> > > > 5.1.1. [Building QVM](#building-qvm)<br/>
> > > > 5.1.2. [MSYS2 (mingw) (Building dynamic libraries (.dll))](#msys2-mingw-building-dynamic-libraries-dll)<br/>
> > > > 5.1.3. [Cygwin (mingw) (Building dynamic libraries (.dll))](#cygwin-mingw-building-dynamic-libraries-dll)
> > >
> > > 5.2. [Linux](#linux)
> > > > 5.2.1. [Building QVM](#building-qvm-1)<br/>
> > > > 5.2.2. [Building shared libraries (.so)](#building-shared-libraries-so)
> > >
> > > 5.3. [Optional](#optional)
> > 6. [License](#license)

# TODO list:

- [x] ~~Toggeable flight (bind key). Hint: FLIGHT POWERUP~~
- [ ] Gauntlet logic must be replaced as a bind key, it will use fight animation (using kicks and fists)
- [x] ~~Remove weapon visuals (models and stuff)~~
- [x] ~~Animations as listed on the old docs~~
- [x] ~~Bind key to recover ki energy~~
- [ ] Bind key to toggle speed (ki boost). HINT: HASTE POWERUP
- [x] ~~Replace ammo to ki energy stamina~~
- [x] ~~Third person traceable crosshair~~
- [ ] Make ki energy regeneration, ki use, attacks, charging balance indicated on old docs
- [ ] Powerlevel and Power Tiers indicated on old docs
- [ ] Hit Stun (makes player can't move, attack, block or charge)
- [ ] Power Struggles (when two beam attacks collide)
- [ ] Blocking (consumes ki energy, transfers all damage to ki instead of health, deflect missile attacks, more info on old docs)
- [ ] Short-Range Teleport (when pressing 2 times left or right)
- [ ] Transformations (related to Power Tiers)
- [ ] Attacksets (configurable for cfgs)
- [ ] Skin Config File (explosionModel, explosionShader, missileRotation, missileShader, ... look old docs about that. "Custom plugin models")
- [ ] Playable third person mode and first person vis mode (add the options in the UI Setup menu)
- [ ] Cvars as described on old docs
- [ ] Last Man Standing gamemode
- [ ] Survival gamemode
- [ ] 6 different selectable characters, each with 5 attacks (can be referenced to some previous tasks)
- [ ] 21 different ki attacks including controllable, homing, and chargeable attacks (no guns) (can be referenced to some previous tasks)

### History

![image](https://user-images.githubusercontent.com/49716252/186024989-69bc7473-d82e-4adf-b11f-dacfba4625dd.png)

Started: 1999 <br/> 
Ended: 2002

Bid For Power is a total conversion for QuakeIII that plays nothing like the original game. Players take control of Ki-powered superheros and battle it out in a mostly aerial fight. The game is highlighted by the work of a great art team and an original style, and the gameplay is extremely fast paced. It can be difficult to keep up with until you get the hang of it.

The source code was lost, but the assets and some docs are in any place.

- Old dev journal: https://web.archive.org/web/20020205150340/http://www.bidforpower.com/journals/yrgol.php

Click here to see the [Old dev journal (Markdown edition)](docs/ygorl_dev_journal.md)

### About the repository

We're doing a recreation of the lost source code. <br/>
The highest priority goal is to completely copy BFP game logical structure. <br/>
Any fixes, improvements and contributions are welcome. But we still can't accept secondary things until it's completely finished and becomes stable.

### References and clues to know how should be the game

Documentations, references and extracted stuff will give us clues to reach the goals. <br/>

- Old documentations:

    * [Guide](docs/Guide.md)

    * [Creating custom plugin models](docs/Create_Custom_Models.md)

<br/>

- Cvars, cmd and bind stuff about the old game:

    * [Bindlist](docs/bind_bfp_list.txt)

    * [Cmdlist](docs/cmd_bfp_list.txt)

    * [Cvarlist](docs/cvar_bfp_list.txt)

We can see, e.g. `g_plKillBonusPct` cvar, which means we need to find the function that rewards the player and do something with that. Something like this:

```c
if ( killedSomeone ) {
   pl = currentPL + (currentPL * g_plKillBonusPct);
}
```

- How should the main menu be:

<img src="https://user-images.githubusercontent.com/49716252/188994358-3d03ef80-e27f-4e5e-96ac-bd0200134ced.jpg" alt="main_menu" width=560 />

<br/>

- How should the gameplay menu be (when press Esc to go):

<img src="https://user-images.githubusercontent.com/49716252/188993661-ae80923d-4ec9-47bd-bb65-0b9f036aa241.png" alt="gameplay_menu" width=560 />

<br/>

- How should the HUD be:

<img src="https://user-images.githubusercontent.com/49716252/188994019-589d3046-e553-47de-afff-5fb0eb2dd7ec.png" alt="hud_display" width=560 />

<br/>

- How should UI setup be:

<img src="https://user-images.githubusercontent.com/49716252/188993194-10feb38b-2809-41d1-9e81-1ce96ff1428a.png" alt="ui_setup" width=560 />

<br/>

- How should UI player settings be (keep in mind all players use running animation):

<img src="https://user-images.githubusercontent.com/49716252/188993401-9e7036e6-cbed-47ac-8aea-431963a343bf.png" alt="ui_player_settings" width=560 /><br/>
<img src="https://user-images.githubusercontent.com/49716252/188994108-c5bdfa06-32d1-4ebe-beb1-d74e9e6159ac.jpg" alt="ui_player_settings2" width=560 />

<br/>

- How should UI controls be:

<img src="https://user-images.githubusercontent.com/49716252/188993792-fd9c4440-3b18-45bf-9964-598459695fd9.png" alt="ui_controls" />

<br/>

- How should UI BFP options be:

<img src="https://user-images.githubusercontent.com/49716252/188993293-db531ff5-5754-4ded-99ad-b5f3b2f5fd72.png" alt="bfp_options" width=560 />

<br/>

- When player receives a hit stun (`g_hitStun 1`):

<img src="https://user-images.githubusercontent.com/49716252/188993948-435cec9c-84a4-477a-b072-1740ca4f1f7d.jpg" alt="hit_stun_received" width=560 />

<br/>

- When player is being ready to shot (holding a key):

<img src="https://user-images.githubusercontent.com/49716252/188993564-d1d5d9ee-80e3-4c15-b0c4-eb966441e57d.png" alt="ready_to_attack" width=560 />

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

BFP config (general binding and some client stuff, unused. WARNING: when executing, game crashes):

   * [bfp.cfg](cfgs/bfp.cfg)

Other q3 config:

   * [q3config.cfg](cfgs/q3config.cfg)


### How to build

- ### Windows:

    * #### _Building QVM_: 

    Execute `build.bat` to compile qvms, keep in mind you must be in the repository directory.

    Once compiled successfully, look for `pak9.pk3`, copy and paste into `baseq3/` or mod Q3 game directory.

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

    5. Compile with make
    ```sh
    make ARCH=x86_64
    ```
    32-bit:
    ```sh
    make ARCH=x86 WINDRES="windres -F pe-i386"
    ```

    7. Find the dlls in `build/release-mingw64-x86_64`, for 32-bit: `build/release-mingw32-x86`. <br/><br/>

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

- ### Linux:

    * #### _Building QVM_: 

    The alternative to execute and get the compiled qvms with `build.bat` requires [`wine` package](https://www.winehq.org/). So, in that part, needs the i386 package:
    ```sh
    sudo dpkg --add-architecture i386 && sudo apt-get update && sudo apt-get install wine32-development
    ```
    But it could be executed without using 32-bit package, if your system supports 64-bits. Go to WineHQ page anyways.
    
    Keep in mind, you must be in the repository directory to execute the script:
    ```sh
    wine cmd /c build.bat
    ```

    Once compiled successfully, look for `pak9.pk3`, copy and paste into `baseq3/` or mod Q3 game directory.

    NOTE: until now, there's no fully compileable QVM tool found for Linux.

    * #### _Building shared libraries (.so)_:
    
    If you don't have gcc tools, install the build-essential packages, which is also known as a meta-package, it contains the GCC compiler all the other essentials used to compile the software written in C and C++ language.
    Also, requires `libc6-dev-i386` for x86 builds and `g++-multilib` for cross-compiling.
    ```sh
    sudo apt-get install build-essential libc6-dev-i386 g++-multilib
    ```

    Simply execute: 
    ```sh
    make
    ```
    And find .so files in `build/release-linux-x86_64`, for 32-bit: `build/release-linux-x86`. <br/><br/>


- ### Optional:

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

<br/>
**IMPORTANT NOTE**: This repository was initialized from https://github.com/marconett/q3a.

#### Currently non-used source code files and not in the build tools:

- cg_particles.c
- ui_rankings.c
- ui_rankstatus.c
- ui_signup.c
- ui_specifyleague.c
- ui_spreset.c

### License

The mod source code is GPLv3 licensed, the source code contents are based on Quake III Arena which is GPLv2 licensed.

## Credits

Bid For Power is made by these staff members. We don't own materials such as art designs, maps and character models from their packages.

#### Bid For Power Staff Members	

Ansel<br/>
Skin Artist<br/>

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
