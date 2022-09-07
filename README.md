Bid For Power (BFP) source code
===============================

A legendary Quake 3 Arena mod from the 90's era.

<p align="center">
<img src="https://user-images.githubusercontent.com/49716252/186026745-458849b2-385b-4108-8881-e55e82fb1493.png" alt="zeqlogo" width=320 />
</p>


## **WARNING!** UNDER CONSTRUCTION!

# TODO list:

- [x] ~~Toggeable flight (bind key). Hint: FLIGHT POWERUP~~
- [ ] Gauntlet logic must be replaced as a bind key, it will use fight animation (using kicks and fists)
- [ ] Remove weapon visuals (models and stuff)
- [ ] Animations as listed on the old docs
- [ ] Bind key to recover ki energy
- [ ] Bind key to toggle speed (ki boost). HINT: HASTE POWERUP
- [ ] Replace ammo to ki energy stamina
- [ ] Powerlevel and Power Tiers as indicated on the old docs
- [ ] Hit Stun (makes player can't move, attack, block or charge)
- [ ] Power Struggles (when two beam attacks collide)
- [ ] Blocking (consumes ki energy, transfers all damage to ki instead of health, deflect missile attacks, more info on the old doc)
- [ ] Short-Range Teleport (when pressing 2 times left or right)
- [ ] Transformations (related to Power Tiers)
- [ ] Attacksets (configurable for cfgs)
- [ ] Skin Config File (explosionModel, explosionShader, missileRotation, missileShader, ... look old docs about that. "Custom plugin models")
- [ ] Playable third person mode and first person vis mode (add the options in the UI Setup menu)
- [ ] Cvars as described on the old doc
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

### About the repository

We're doing a recreation of the lost source code. <br/>
The highest priority goal is to completely copy BFP game logical structure. <br/>
Any fixes, improvements and contributions are welcome. But we still can't accept secondary things until it's completely finished and becomes stable.

### References and clues to know how should be the game

Documentations, references and extracted stuff will give us clues to reach the goals. <br/>

- Old documentations:

- - [Guide](docs/Guide.md)

- - [Creating custom plugin models](docs/Create_Custom_Models.md)

<br/>

- Cvars, cmd and bind stuff about the old game:

- - [Bindlist](docs/bind_bfp_list.txt)

- - [Cmdlist](docs/cmd_bfp_list.txt)

- - [Cvarlist](docs/cvar_bfp_list.txt)

We can see, for example, `g_plKillBonusPct`, which means we need to find the function that rewards the player and do something with that. Something like this:

```
if ( killedSomeone ) {
   pl = currentPL + (currentPL * g_plKillBonusPct);
}
```

- How should be the main menu:

<img src="" alt="main_menu" />

- How should be the gameplay menu (when press Esc to go):

<img src="" alt="gameplay_menu" />

- How should be the HUD:

<img src="" alt="hud_show" />

- How should be UI setup:

<img src="" alt="ui_setup" />

- How should be UI player settings (keep in mind all players use running animation):

<img src="" alt="ui_player_settings" /><br/>
<img src="" alt="ui_player_settings2" />

- How should be UI controls:

<img src="" alt="ui_controls" />

- How should be UI BFP options:

<img src="" alt="bfp_options" />

- When player receives a hit stun (`g_hitStun 1`):

<img src="" alt="hit_stun_received" />

- When player is being ready to shot (holding a key):

<img src="" alt="ready_to_attack" />

<br/>

- **cfg files**:

A sample inside models/players/player_name/default.cfg:

- - [default.cfg](cfgs/default.cfg)

Server config:

- - [bfp_server.cfg](cfgs/bfp_server.cfg)

Attacksets:

- - [bfp_attacksets.cfg](cfgs/bfp_attacksets.cfg)

Weapon settings:

- - [bfp_weapon.cfg](cfgs/bfp_weapon.cfg)
- - [bfp_weapon2.cfg](cfgs/bfp_weapon2.cfg)

BFP config (general binding and some client stuff):

- - [bfp.cfg](cfgs/bfp.cfg)

Other q3 config:

- - [q3config.cfg](cfgs/q3config.cfg)


### How to build

- Windows:

Execute `build.bat` to compile qvms, keep in mind you must be in the repository directory.

Once compiled successfully, look for `pak9.pk3`, copy and paste into `baseq3/` or mod Q3 game directory.

<br/><br/>

Note: This repository was initialized from https://github.com/marconett/q3a.

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
