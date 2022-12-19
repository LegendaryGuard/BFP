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
- [x] ~~Remove weapon visuals (models and stuff)~~
- [x] ~~Animations as listed on the old docs~~
- [x] ~~Bind key to recover ki energy~~
- [ ] Bind key to toggle speed (ki boost). HINT: HASTE POWERUP
- [x] ~~Replace ammo to ki energy stamina~~
- [x] ~~Third person traceable crosshair~~
- [ ] Make ki energy regeneration, ki use, attacks, charging balance as indicated on the old docs
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

Click here to see the [Old dev journal (Markdown edition)](docs/ygorl_dev_journal.md)

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

<img src="https://user-images.githubusercontent.com/49716252/188994358-3d03ef80-e27f-4e5e-96ac-bd0200134ced.jpg" alt="main_menu" width=560 />

<br/>

- How should be the gameplay menu (when press Esc to go):

<img src="https://user-images.githubusercontent.com/49716252/188993661-ae80923d-4ec9-47bd-bb65-0b9f036aa241.png" alt="gameplay_menu" width=560 />

<br/>

- How should be the HUD:

<img src="https://user-images.githubusercontent.com/49716252/188994019-589d3046-e553-47de-afff-5fb0eb2dd7ec.png" alt="hud_display" width=560 />

<br/>

- How should be UI setup:

<img src="https://user-images.githubusercontent.com/49716252/188993194-10feb38b-2809-41d1-9e81-1ce96ff1428a.png" alt="ui_setup" width=560 />

<br/>

- How should be UI player settings (keep in mind all players use running animation):

<img src="https://user-images.githubusercontent.com/49716252/188993401-9e7036e6-cbed-47ac-8aea-431963a343bf.png" alt="ui_player_settings" width=560 /><br/>
<img src="https://user-images.githubusercontent.com/49716252/188994108-c5bdfa06-32d1-4ebe-beb1-d74e9e6159ac.jpg" alt="ui_player_settings2" width=560 />

<br/>

- How should be UI controls:

<img src="https://user-images.githubusercontent.com/49716252/188993792-fd9c4440-3b18-45bf-9964-598459695fd9.png" alt="ui_controls" />

<br/>

- How should be UI BFP options:

<img src="https://user-images.githubusercontent.com/49716252/188993293-db531ff5-5754-4ded-99ad-b5f3b2f5fd72.png" alt="bfp_options" width=560 />

<br/>

- When player receives a hit stun (`g_hitStun 1`):

<img src="https://user-images.githubusercontent.com/49716252/188993948-435cec9c-84a4-477a-b072-1740ca4f1f7d.jpg" alt="hit_stun_received" width=560 />

<br/>

- When player is being ready to shot (holding a key):

<img src="https://user-images.githubusercontent.com/49716252/188993564-d1d5d9ee-80e3-4c15-b0c4-eb966441e57d.png" alt="ready_to_attack" width=560 />

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
