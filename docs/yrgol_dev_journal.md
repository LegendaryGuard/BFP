
# BFP 1.1 Update Information - 02/04/2002 by Yrgol	

The grand list of my updates to the latest build of Bid For Power. Enjoy.

BFP 1.1 Changes

features:

* added a bfp_server.cfg file, which lets server ops control:

- flight cost, boost cost, ki regen, and ki charge regen.

* aura color is now forced to red or blue in team games depending on team.

* new console.

* tweaked attack shaders for lighting values.

* new fingerblast look and sound.

* lighter versions of the maps gptourney1 and gpctf1.

* added cone of fire support to the attack scripting.

balance:

* lowered boost cost significantly

* upped ki regen rate so that flying at max pl costs nothing.

* fixed the extra knockback on attacks that use that feature. tornado blast uses this.

* rebalance of most attacks.

bugs:

* fixed the 1v1 server crash. 1v1 servers should be as stable as the other game types now.

* fixed handling of plugin player models on clients that don't have the model.

* fixed a "weapon number out of range" message followed by a crash when playing with bots.

* fixed the appearance of explosion spawn attacks (homing special multi-beam bug).

* made suicide take away a kill.

* fixed losses being assigned to the wrong player in 1v1 games.

* ctf servers show up in 1.0 as lms servers. fixed.

* survival servers show up in 1.0 as single player servers. fixed.

* gametype in 1.0 gets cut off in the servers list. fixed.

* fixed missing friend shader for team games.

* added dummy player sounds for tetsedah, ryuujin, and pyrate to get rid of some warning messages.

* fixed the small own aura setting.

* switched the missile dlights of the two razor disks.

* fixed tornado blast catching attacks that have higher priority.

* fixed ki use toggle.

* fixed position of sparks in power struggles.


 

# more stuff for the patch - 01/23/2002 by Yrgol	

* fixed the 1v1 server crash. 1v1 servers should be as stable as the other game types now.
* lowered boost cost significantly
* upped ki regen rate so that flying at max pl costs nothing.
* fixed a "weapon number out of range" message followed by a crash when playing with bots.
* fixed the extra knockback on attacks that use that feature. tornado blast uses this.
* fixed losses being assigned to the wrong player in 1v1 games.

 

# bugs so far - 01/20/2002 by Yrgol	

I'm waiting for a decent amount of bugs to show themselves before doing a patch.

fixed:
* added a bfp_server.cfg file, which lets server ops control: flight cost, boost cost, ki regen, and ki charge regen.
* fixed the appearance of explosion spawn attacks (homing special).
* aura color is now forced to red or blue in team games depending on team.
* ctf servers show up in 1.0 as lms servers. fixed.
* survival servers show up in 1.0 as single player servers. fixed.
* gametype in 1.0 gets cut off in the servers list. fixed.
* fixed missing friend shader for team games.
* added dummy player sounds for tetsedah, ryuujin, and pyrate to get rid of some warning messages.

known and not fixed yet:
* there seems to be some stability issues with 1v1 servers. this mainly seems to happen on map changes.


 

# patch - 01/19/2002 by Yrgol	

The next thing released by bfp will be a dbz plugin media pack or two. This will be an optional download that includes the old models in plugin form and some new maps.

After that, the next releae of bfp will be a patch that will arrive in about two weeks to a month or so from now. This patch will have a decently small filesize, and it's for fixing whatever bugs show up. If you encounter a bug, please visit the bugs forum to post a message so I can fix it. Once most of the bugs are gone I'll start working on new attacks with some real variety now that my hands aren't tied by dbz.

The homing special has a graphical glitch. Once it spawns the multiple homing missiles, those missiles appear to each player as his own 4th tier attack instead of appearing as the attack that it is supposed to. This has no effect on how the attack actually operates and does damage, but it can get confusing. This is already fixed for the next version.


 

# Modding BFP - 12/13/2001 by Yrgol	

It is possible to modify bfp in a number of ways. Some of them are compatible with the base bfp, and others are destructive to the base bfp. The only type of bfp modding that I will support is the compatible type.
Multiple groups have tried to make their own versions of various hacked copies of bfp, so in order to prevent players from having to choose between versions, I'm posting a little guide on how to make mods of bfp coexist with each other. This is not supposed to be a comprehensive guide on how to mod bfp, just some dos and don'ts if you decide to modify it.

Model and Map Packs:<br/>
If you only want to create some new models that use existing attack sets, or new maps to play on, create model and map pk3s and release them together so the players can install them in their base bfp directory. These models and maps will be selectable in the menu just like any other model or map. BFP allows for a significant amount of variation in plugin models.

Game Balance:<br/>
If you are unhappy with the balance of the game and run a server, just modify the script file bfp_weapons.cfg to suit your tastes. You can then start up your server and people using the base bfp will be able to play there. I will be adding more ability to do this in the future, but what already exists is pretty significant.

New Attacks or Attack Sets:<br/>
If you wish to add new attacks or attack sets to the game, you will have to create a full mod of bfp. This is because otherwise, players will accidentally connect to your bfp mod using the base bfp, and their game is likely to crash. More importantly, I don't want players to have to randomly try bfp servers until they can find one that is compatible with the version they are using.

The server-finding code in the game will find any server running any mod that lives in a directory that starts with "bfp", and display the full name of that mod. The official bfp files will be in the "bfpq3" directory. Unnoficial mods of bfp can go in directories like "bfpmyversion" or "bfpcool". Players will be able to see "bfpcool" or "bfpq3" in the menu, so that they can find a server compatible with the version of bfp they are running.

Rule #1:<br/>
Never add or remove anything to any file named similarly to bfp0.pk3 in the base bfp directory. This will invalidate sv_pure, and it will prevent me from patching the game! You can use quake 3's pk3 indexing to override files in bfp0.pk3 without touching that file.

Rule #2:<br/>
Nobody is getting ahold of my source code for quite a long time, so don't bother asking. If you wish to add new code to bfp, you will have to code the entire game from scratch.

Rule #3:<br/>
Do not include any official bfp model, map, or image files with your mod. Free game or not, enforcable or not, doing so is a violation of copyrights and is also totally unecessary. Make use of quake3's base mod ability. Quake 3 allows you to create a mod that uses files from another mod's directory. If you do not want your players to have to download the official bfp, you will have to create all new maps and models to use for your game.


 

# weapon limit - 12/07/2001 by Yrgol	

Quake3 has a limit of 16 different weapons. BFP used to have a limit of 31 total attacks, with 21 of those being used. Thanks to about 2000 lines of code changes, BFP now has a practically unlimited max number of attacks.
I currently have slots for 99 different attacks that can be added without any code, but I could easily double or triple that if bfp ever comes close to filling up all 99 attacks.


 

# mp3 support - 11/30/2001 by Yrgol	

I added Tim "Timbo" Angus' mp3 to wav converter into bfp. It can be found at http://tremulous.sourceforge.net/junk/cg_mp3decoder/

This will allow us to include more music in the download, since each music file will have a smaller filesize.

This will not allow you to play your own mp3's. It is not an mp3 player, it is an in-game utility to convert mp3's into a q3-playable format.


 

# Client-side scripting - 11/29/2001 by Yrgol	

I've finished the client side weapon scripting, and I've posted a doc in the editing and modifications forum describing how to use the scripting in order to create plugin models for BFP.<br/>
You can find the post here:
http://www.bidforpower.com/forum/showthread.php?s=dc60acd92fe3f101aff5ad112f78d25d&postid;=508160#post508160


 

# 1.30 and bots - 11/19/2001 by Yrgol	

Oddly enough, the 1.30 point release fixed bot flight, which the 1.17 point release broke. The bots have no problem flying up or down anymore. The bots switch weapons now as well, and I added a few other bot tweaks.
Bfp bots are pretty good for deathmatch practice. They're working much better than I expected. I'm sending a big woot out to the 1.30 point release for q3.


 

# - 11/17/2001 by Yrgol	

What we are currently working on is equivalent to beta 2 for most mods, or in bfp jargon phase 1. We had phase 1 completed for dbz quite a long time ago, and then the funimation/infogrames stuff happened. Most of the time since then has been used for converting bfp to not use dragonball z copyrights.

The code is ready to go, the ui images have been changed to be legal, and we have several non-dbz maps finished. I currently have 3 of the 6 necessary default models. When I have all 6 default models, I will create a full build for the team to test. If nothing horrible goes wrong we'll probably release about a week after that.

I've been working on scripting much of bfp. When this is complete, new characters and attacks will not require any additional code. This will allow team members and the community to create new unique characters for future releases. There will be a second release which will likely include more maps and models, some more in-depth gameplay modes, more attacks, and a completed attack scripting system. I will also likely be replacing the melee system with something different.

I've added much more powerful custom plugin model support. The old dbz models will be able to perform identical to what they did before, provided they are converted to the new format. I have repackaged the old bfp models and included the necessary script files for them to work. I will be working on making the support for customizations much more significant, but that will be for future releases.

I added server side weapon scripting. It allows a large amount of control over the game balance by the server operators, and makes my job of balancing much easier. It can not yet be used to create new attacks, but it will eventually. While doing this I fixed up a few serious gameplay complaints.

Direct hit detection has been made more reliable. This makes the kienzan equivalent and the deathball easier to use.

Explosion damage is no longer all or nothing. The damage decreases as distance from the explosion increases like in baseq3.

Attack radius has been separated from explosion radius. The attack radius and explosion radius for each of the attacks have been tweaked.

The knockback physics for attacks and explosions has been seriously tweaked. Attacks can now be used to rocket jump.

Boost jumping has been toned down. It was simply too excessive before.

Another total rebalance of all attacks happened.


 

# scripting - 10/26/2001 by Yrgol	

There's no way I can get all of this done before release, but this is the direction I'm going in.

I'm working on attack scripting for servers only. This will allow new attacks of any rough type that bfp already contains to be added to the game with little or no code. I have a script file set up and partially implemented that has like 30 variables for each of the attacks. This will not allow models to define new attacks, but it will allow team members to work on attacks without me, or mods of bfp. It also lets server ops who hate the game balance to tweak it significantly.

I have plans to add a server script file to define what the attack sets are, and even how many different attack sets there are. This config file will make adding new attack sets for existing attacks a trivial task. The file will look something like this:

```
attackset 1
attack1 15
attack2 20
attack3 21
attack4 2
attack5 30
defaultmodel models/players/goku
```

Per-skin config files have been added that allow things like attack names and voices to be defined by the skin. I have plans to move a lot of stuff into these config files, including attack shaders, sounds, and models. The individual models will never be able to define how the attacks operate, only how they are displayed to the user.

I will eventually create a separate server config file that defines all the ki costs for flight and things along those lines.

I've been working on the server attack config scripting for about a week, and it's a huge job. I can not finish all of this in a reasonable time frame, but eventually bfp will be close to just being an engine with the game all defined by runtime scripts. Once that point is reached I'll no longer be the bottleneck for a lot of the future work, and I can concentrate on gametypes and random stuff.

I do not expect much of this to be implemented enough to matter before we get to release though. I'd much rather release asap and then improve.


 

# Conversion - 09/10/2001 by Yrgol	

With the exception of the names, the attacks are not changing. Most of the maps are not changing either.
The new thrust of development for me is moving as many things as reasonable into the player models. Custom player models can be made for any of the sets of attacks, and I'm working on a way for skins to define the names of the attacks. I don't want to let each skin pick the attacks to use for balance, memory, and implementation reasons. The voices that get played when using certain attacks have been moved into the player model.

The bfp leak is bfp's biggest competition right now. The release has to be better than the previous stuff. There are a few annoying bugs to work out, and some of the top tier attacks throw balance way off.

My job has been sapping most of my time and motivation to program lately.


 

# HoverQ3 - 08/17/2001 by Yrgol	

I released a little mod called Hover Q3. It includes some art from Pyrofragger, Kit Carson, and Sole. You can find it at http://www.bidforpower.com/yrgol/hoverq3

It's a 3 meg download and is a lightweight but fun mod. Don't expect anything like bfp from this, but it's well worth checking out.


 

# destructable objects - 07/08/2001 by Yrgol	

Breakable map entities are in, with multiple stages of breaking. I havn't done anything fancy with particles and destructable objects yet. None of the maps currently use this feature.

 

# stuff - 07/06/2001 by Yrgol	

custom models are fully supported now. if you create a hulk hogan model and put it in /models/players/vegetahogan/, then the hulk hogan model will show up in the ui as a selectable skin for vegeta, brotha. I'm gonna hit you with a final flash in a steel cage.

beams are bendy now. beams are not guided missiles, they are a beam of energy that gets longer quickly. when you turn left while firing, the beam bends to the right briefly before straightening out again.

other than that, some random bugfixes in the animation playing and 1.29 fixes.


 

# <-- that way - 06/21/2001 by Yrgol	

* fixed animation jitter caused by jump anims that shouldn't be played.
* made it play the run anim faster during boost run.
* fixed the bug that made the ki trail show up behind objects.
* gave the accurate third person crosshair some smoothing to prevent it jumping around.
* added a toggle to use a stable crosshair instead of the accurate one.
* fixed a bug that made killing yourself give you a PL boost.
* made renzoku cost a ki pctage to prevent link overflow
* fixed a cheat that let people get vegeta's attacks with a piccolo model.
* enabled custom player models.

I have a real job, and right now it's very demanding. When I have time and energy for bfp, I'm trying to polish as much as I can.

The custom player models are weakly supported. Just for example, if you wanted to make a gundam and put it in bfp with vegeta's attacks, you'd have to create a model and skin in models/players/vegetagundam. Once there, it acts as a vegeta skin, even though it's actually a skin and a model. If you were playing with someone who didn't have your gundam model, they'd see the default vegeta.

I'll release a far more descriptive doc on how to make a custom bfp model later on when it matters. BFP uses a lot of animations.


 

# random update - 05/29/2001 by Yrgol	

had a somewhat productive week. bfp comes in spurts.

updates:
* accurate 3rd person crosshair.
* particle dust trail for boost running or flying close to the ground.
* particle bubble effects that float to the surface and gather there.
* particle splash + bubbles for entering water.
* particle bubble trail for boost swimming:
<img src="https://user-images.githubusercontent.com/49716252/188996461-499a7aed-be3d-4cd7-8322-67ac328ec0c1.png" alt="particlebubble" width=640 />

* particle bubble trail for boost flying close to water.
* particle bubble effects for charging underwater:
<img src="https://user-images.githubusercontent.com/49716252/188996690-87659342-a9e3-4007-bea9-9c1ba03aa253.png" alt="underwatercharge" />

* fixed the waterdance bug.
* lowered zanzoken cost.
* added antigrav rock particles for charging:
<img src="https://user-images.githubusercontent.com/49716252/188996766-7c0055fb-2c3a-4cdf-b188-5d70fabd8d69.png" alt="groundcharge" width=640 />

* the antigrav rocks fall to the ground when you stop charging.
* gave rock particles collision detection and bouncing properties


 

# survival mode - 04/08/2001 by Yrgol	

I added (past tense) survival mode to the game. This might morph into budokai mode when mappers and I both get some time. The name comes from the mode found in a lot of fighting games.

Survival mode is basically 1v1 where if you die once, you go to the end of the line. The winner does not get healed in between matches, and people spawning in get power level based on the average like in FFA mode. Spectator scores are saved and visible. The winner is the first person to reach the frag limit.

1v1 mode is meant for challenge matches. It only works well with about a 20 frag limit. Survival mode is intended to give a 1v1 option where you don't have to wait around for 20 minutes before playing. I hope it finds a home on some servers, because it's the mode I most want to play.


 

# sticky keys and fps dependencies - 04/07/2001 by Yrgol	

Melee, block, ki boost, and ki charge had a tendency to get "stuck" in online play when the wrong packet got lost. I changed these keys so that they communicate with the server in the same way firing a weapon does. They shouldn't get stuck anymore even when there's a lot of packet loss. This is a bug that's been present since build 1. I'm glad to be rid of it.
Flight speed was highly dependent on FPS as of rc2. Ki boost cost was highly dependent on FPS since build 1. Both of these are fixed now also.


 

# subject? - 04/02/2001 by Yrgol	

From memory since I havn't been keeping close logs lately.

* got a new job
* moved from pittsburgh to boston
* added particle rocks and smoke to explosions.
* added a particle trail to the particle sparks.
* added particle bubbles to attacks underwater.
* added some particles to power struggles.
* redid the shaders so they don't get overbright.
* simplified the bfp options menu by making explosion effects have a selecter between 4 different levels of intensity.
* made kakusan fire a variable number of balls depending on the amount of time spent charging it. fires from 2 to 6 homing balls. slowed the charge time, fixed the angle of fire on the homing balls.
* made ki drain a percentage for eyebeam, shyogeki ha, and kiaho.
* made kiaho a constant fire attack. decreased the range massively. decreased the knockback somewhat. made it bounce all missile attacks except for deathball and kienzan.
* eyebeam is NOT underbalanced in the slightest in its current form.
* increased the speed of all beam attacks.
* made the menu look prettier.
* added 2 new maps, a giant sound pack from 17, a new console font from gp, a few random things from dash.
* fixed body transformations.
* tried out bfp at 640x480 with the options all turned up on a p3 450 running a TNT ONE 16 MEG VIDEO CARD and got 20-30 fps on danorthern against 3 bots.
* create an installer exe for bfp.


 

# particles - 02/16/2001 by Yrgol	

I added some particle effects to the game. I havn't really figured out what to do with them yet, but they are there. Just as a start I added the generic explosion sparks. As with every other effect, if you don't like it you can turn it off in the menu.

<img src="https://user-images.githubusercontent.com/49716252/188996981-5d1d1596-bf19-448c-9f80-819a634ee117.png" alt="bunch of renzokus fired at a wall" width=640 />


 

# Old style update - 02/08/2001 by Yrgol	

changes since the thing we sent to the news sites:
* lowered renzoku cost to 100
* increased renzoku missile speed to 8000.
* increased renzoku radius to 120.
* lowered renzoku firing speed to 200ms.
* put in a 1 second delay before you can zanzoken out of stun.
* increased SBC radius to 150.
* increased SBC damage from 35%*mult to 40%*mult.
* increased mouthbeam damage from 20% to 30% (effective 60% vs same PL).
* lowered final flash damage from 40%*mult to 25%*mult.
* increased the angry gohan range from 500 to 1000.
* tweaked the angry gohan aura scaling.
* lowered angry gohan ki cost from 200 to 150.
* fixed a bug that made other people's charging flashes not show up.
* fixed the lagometer position
* made an exception so the death ball can compete with beams in collisions.
* fixed a bug that made third person extend into spectator free mode.
* fixed a horrible bug that made players not cycle in 1v1.
* tweaked some LMS code.
* fixed a bug that made the blocking anim play a bit too long.
* fixed the problem with the angry gohan shader.
* added basic power struggles.
* redid kiaiho.
__
* took away boost speed bonus while _firing_ a beam.
* lowered kamehameha damage from 40%*mult to 30%*mult.
* upped final flash damage from 25%*mult to 30%*mult.
* made deathball work in power struggles.
* put the angry gohan cost back the way it was.
* lowered angry gohan range to 800 from 1000.
* slowed recharge time to 6 seconds.
__
* added the new gohan and krillin.
* added some attack charge sounds.
* redid the timing on masenko and gave it a HA sound.
* fixed a bunch of post-death bugs.
* removed the generic flash sounds for kamehameha and masenko.
* added a new sprite for final flash.
__
* added sprites for the big bang, sokidan, masenko, kamehameha, and renzoku.
* made oozaru picking more random.
* fixed the oozaru extendo penis.
* totally redid flight physics.
* changed up and down orientation while flying to be relative to the player's axis.
* added cg_flytilt for tilting the viewpoint when boost flying.
* fixed a bug with zanzoken going through clip brushes.
* increased the size of the smoke ring around explosions.
* added cg_explosionSmoke to turn off the smoke ring.
* redid jump.
* added wall jumps.
* added a short delay between jumps excluding wall jumps. avoiding people looking like bunnies.
* fixed the "slightly sticky ground" flight bug.
* added ctf, but it's not selectable from the menu.
* added more dramatic transformations. they are 5 seconds long and you are invuln during it.


 

# Random updates - 02/04/2001 by Yrgol	

Ansel's been making a lot of sprites to replace some of our attack models. Flight physics was totally redone to include momentum.

CTF mode was added back in, but I don't know if it's going to be a fully supported mode or not. CTF is kinda goofy when you can fly fast, and we don't have any maps for it. CTF should not be confused with capture the dragonballs, which still isn't in phase 1.


 

# Power Struggles - 01/10/2001 by Yrgol	

Power struggles are in phase 1 now. Ki recharge time has been increased to 6 seconds for now, subject to change at any given moment. We had it take about 20-30 seconds a long time ago and it really killed the gameplay. Lots of attacks have gone through some rebalancing.

 

# Yrgol’s Important Bid For Power Phase 1 README!!! - 01/10/2001 by Yrgol	

(complete with 3 different exclamation points!)
A wise man once said: “Those who do not read this document will suck at BFP.”

What is Bid For Power?
Bid For Power (BFP) is a quake 3 total conversion. We have taken quake 3 and used it to create a new game. In order to play BFP, you will need the full retail version of Quake 3 Arena by ID Software, but once you have that, our game is a free downloadable add-on.

In phase 1 of BFP, we place 6 of the fighters from the Freeza saga in an arena in order to fight. Power is gained over time, but the way to get power quickly is to kill other players. Obviously in the show, Krillin would not be able to fight Freeza, but if we followed that rule who would play Krillin? In BFP, the only difference between the characters is the set of attacks available.

Features:
- 6 different selectable characters, each with 5 attacks. Goku, Piccolo, Vegeta, Freeza, Gohan, and Krillin.
- 21 different ki attacks including controllable attacks, homing attacks, and chargeable attacks (no guns!).
- Toggleable flight.
- offhand melee combat
- Re-"charge"able ki energy for ammo, flight, and speed/power boost.
- Power level determines the strength of attacks, speed of movement, max health, max ki, ki charge rate, and what attacks are available.
- 5 different game modes: FFA, 1v1, Team DM, Last Man Standing Team DM, and Oozaru mode.
- Numerous new player animations.
- Plenty of cool effects: head and full body transformations, selectable aura, ki trail, scalable attacks, big explosions, playable third person mode, first person vis mode.

Installation (windows):
- Install the full retail version of Quake 3 Arena. The demo version will NOT work with BFP.
- Install the 1.27 point release for Quake 3 Arena, found at http://www.quake3world.com/files/patch127g.html
- Unzip the bfprc1.zip file into your quake 3 arena directory. Make sure to use folder names when extracting the files.
- The directory "bfp" will be created as a subdirectory of the quake 3 directory.
- To start the game, either use the "Mods" tab in quake 3, or start the game with command line args “+set fs_game bfp”. Creating a shortcut to quake 3 with that argument is the recommended way of starting BFP.

Starting BFP:<br/>
There are two ways to start BFP. The easiest way is to use the mods menu in Quake 3. From the main Quake 3 menu, click "mods". Then find "Dragon Ball Z: Bid For Power" on the list and highlight it. If you have a lot of mods installed, you may need to scroll the list in order to make BFP visible. Once you have BFP highlighted, click on the load button and BFP will start.

The other way to start BFP is to use the command line arguments `+set fs_game bfp`. An easy way to do this under Windows is to create a shortcut. Highlight the quake3 icon in whatever directory you installed Quake 3 into, and press ctrl-c to copy the link. Press ctrl-v to create a shortcut to quake3. Right click on the new shortcut and go into the properties menu. Under the `shortcut` tab of the properties menu, change the target to `C:Quake 3 Arenaquake3.exe +set fs_game bfp` if you installed Quake 3 into C:Quake 3 Arena. Use this new shortcut to start BFP.

BFP loads its own config file called 'bfp.cfg'. This allows either way of starting BFP to work without any config file problems.

What is Phase 1?
Phase 1 is the first release of BFP. It covers important characters and places from the Freeza saga of DBZ.

What is Phase 2?
Phase 2 will be the second major release of BFP. It will be released when it is done, and will cover important characters and places from the Android/Cell saga. Phase 2 will also likely have updated effects and game play. There will more than likely be a few patches between phase 1 and phase 2.

How do I play a multi-player game?
This game is designed to be played multiplayer. From the main menu, click on the "multiplayer" button. BFP will scan all of the quake 3 arena servers and display any BFP servers that it finds. Once a BFP server is found, highlight it, click accept and the game will connect to the server.

Is there a single-player game?
Just barely. This game is intended to be played multi-player. A single player game can be played with the bots, but we recommend playing against real people.

Viewpoint
Viewpoint is how you see the game while you play it. You have your choice of 3 different viewpoints for playing BFP. To switch between the view, use the BFP Options menu.

- Third Person: The model is seen from the outside. Unlike the standard quake 3 third person view, in BFP the crosshair is visible, and the angle is changed to create less blocking of the view. Cvars cg_thirdPersonAngle and cg_thirdPersonRange have had their cheat protection removed, and cg_thirdPersonHeight was added for additional customization.

- First Person Vis: First person where your own model is drawn. Your viewpoint is positioned on the model's eyes, and you can see your own arms and legs while fighting.

- First Person: The standard first person quake 3 view.

Power Level

Power level is how strong you are. Power level determines max health, max ki, movement speed, damage caused by attacks, and which attacks are available to use. The only way to get more power is to kill someone. When you die (in most game modes), your power level is reset to the average of all current power levels. When the game ends or the server restarts, the power level of the players is reset to the default.

Power Tiers

Power level is divided into tiers. While most power level effects are based on the power level itself, aura color and attacks available is determined by the tier.

- Tier 1: < 100,000 PL. Blue aura, only one ki attack available.
- Tier 2: 100,000 - 250,000 PL. Red aura, two ki attacks selectable.
- Tier 3: 250,000 - 500,000 PL. Red aura, three ki attacks selectable.
- Tier 4: 500,000 - 999,000 PL. Red aura, four ki attacks selectable.
- Ultimate Tier: 1 mil PL. Yellow aura, all attacks selectable.

Transformations

At 1 million power level, the transformation happens. The aura color turns yellow, and if the character has a transformation it occurs. Saiyajin characters turn into Super Saiyajins. Other than the benefits from increased power level, there is no additional benefit of the transformation.

Ki Energy

Ki is your stamina. Ki is used as ammunition for the ki attacks. Additionally, ki can be used to increase power, speed, jumping height, and special attacks by using the boost key. Ki can be charged in order to replenish it at any time by holding down the charge key. Beware, while charging you can not move or attack, and are highly visible to other players.

Flight

BFP has a toggle able flight button. The default is "f" but it can be set in the controls menu. To begin flying, simply press f once, and once again to stop flying. While flying, you are able to move to wherever you look. The jump key will make you move up, and the crouch key will make you move down. Your ki drains at a steady rate. At low power levels you will need to limit your flight, while at high power levels you can fly all day. A large speed boost is gained by holding down the boost key.

Melee Combat

To engage in melee combat, hold down the melee key and put the crosshairs over your opponent. If you are close range, then you will start beating on him. If you are long range, you will dive at the bad guy for a strike. To add extra knock back and stun your opponent, hold down the boost key while melee fighting.

Hit Stun

While in hit stun, the player can not move, attack, block, or charge. 1 second of hit stun is added when the player runs out of ki. 3 seconds of hit stun is added when melee is used in combination with the boost key. Do not overlook this feature. Being able to knock people into hit stun with ki-boosted melee is one of the keys to being good at BFP.
*The melee stun can be avoided by holding melee while you are attacked.

Zanzoken

Zanzoken is a short, extremely fast movement. In BFP, zanzoken is a controlled short-range teleportation that is performed by double-tapping right or left movement keys. Zanzoken is useful for dodging attacks, and can be used to break out of hit stun after 1 second.

Blocking

Pressing block once grants the player between one and two seconds of blocking. Blocking drains ki quickly, but transfers all damage to ki instead of health. Blocking can also be used to deflect missile attacks. There is a one second delay between when the player stops blocking and when the player can block again.

Ki Attacks

Each character has 5 attacks. At the lowest power tier, the player only has one selectable attack. Each time the player advances to a new power tier, he gets a new selectable attack. By default, each of the numeric keys, 1-5, choose a different attack. Only one attack can be selected at a time, but switching between attacks is instant.

- standard attacks: hold down fire and you fire.
- charge-up attacks: hold down fire to start charging the attack, release fire to fire.
- minimum charge-up attacks: same as charge-up attacks, but with a minimum charge-up time. The ball of ki will appear in the player's hands once he is prepared to fire.
- beam attacks: a type of charge-up attack. Can only fire one beam at a time. After firing, you can aim the attack by moving the mouse, or detonate it by hitting fire again.
- homing attacks: these attacks will home in on the nearest opponent.

Attack Collisions

Unlike quake 3 attacks, BFP attacks are more than just a trajectory. Attacks have actual dimensions, and will explode on contact with players. If an attack looks 50’ wide, it will hit any players who come within 25’ of the center of the attack. Mid-air attack collisions also happen. Two attacks that collide will damage each other, and the stronger attack will cause the other to explode. The exception is any beam attack, which will destroy any other attack that it makes contact with.

Power Struggles

When two beam attacks collide, a power struggle will happen. Instead of the stronger beam blowing up the weaker beam, the stronger beam will push back the weaker beam until it hits the other player. Ki boost can be used to up the power of your beam if you are losing a power struggle. The Death Ball is an exception among the missile attacks, and can be used in a power struggle.

Krillin

- Kikou: standard weak ki blast. Press or hold fire to fire a blast.
- Taiyoken: the solar flare. Hold down fire until charged, then release to blind people around you.
- Sokidan: homing ball. Hold down fire until charged, then release to fire.
- Kakusan: a missile that spawns 4 homing balls when detonated. Hold down fire until charged, and release to fire the missile. Press fire again to detonate the missile into 4 homing balls.
- Kienzan: the destructo disk. Hold down fire until charged, and release to fire. This attack goes through walls and does massive damage.

Vegeta

- Kikou: standard weak ki blast. Press or hold fire to fire a blast.
- Bakuhatsuha: larger ki blast. Press or hold fire to fire a blast.
- Renzoku Energy Dan: multiple fast-firing ki blasts. Hold down fire to attack.
- Big Bang Attack: a beam that has a massive explosion. Hold down fire until charged, release to fire. While firing, use the cursor to aim the attack. Press fire again to detonate into a massive big bang explosion.
- Final Flash: a large and powerful beam attack. Hold down fire until charged, release to fire. While firing, use the cursor to aim the attack. Press fire again to detonate the attack.

Gohan

- Kikou: standard weak ki blast. Press or hold fire to fire a blast.
- Sokidan: homing ball. Hold down fire until charged, then release to fire.
- Renzoku Energy Dan: multiple fast-firing ki blasts. Hold down fire to attack.
- Masenko: beam attack. Hold down fire until charged, release to fire. While firing, use the cursor to aim the attack. Press fire again to detonate the attack.
- Energy Dan (Angry Gohan Attack): Gohan creates a massive and continuous explosion around himself. Press and hold fire. After a few seconds the explosion will appear. Gohan can not move while performing this attack.

Piccolo

- Kikou: standard weak ki blast. Press or hold fire to fire a blast.
- Eye Beam: a continuous blast of power from the eyes. Press and hold fire.
- Renzoku Energy Dan: multiple fast-firing ki blasts. Hold down fire to attack.
- Chobakuretsumaha: a fast and powerful homing attack. Hold down fire until charged, and release to fire.
- Special Beam Cannon: a fast and narrow but extremely powerful spiraling beam. Hold down fire until charged, release to fire. While firing, use the cursor to aim the attack. Press fire again to detonate the attack.

Goku

- Kikou: standard weak ki blast. Press or hold fire to fire a blast.
- Kiaiho: An invisible blast of energy from the hands with a massive knockback. Press or hold fire to attack.
- Sokidan: homing ball. Hold down fire until charged, then release to fire.
- Renzoku Energy Dan: multiple fast-firing ki blasts. Hold down fire to attack.
- Kamehameha: beam attack. Hold down fire until charged, release to fire. While firing, use the cursor to aim the attack. Press fire again to detonate the attack.

Freeza

- Kikou: standard weak ki blast. Press or hold fire to fire a blast.
- Shyogeki Ha: fast firing, extremely fast moving ki blasts from the fingers. Hold fire to attack.
- Freeza Beam: an extremely fast ki attack fired from the finger. Press or hold fire to fire a blast.
- Tsuibi Kienzan: Freeza’s homing version of the kienzan. Hold down fire until charged, and release to fire. This attack goes through walls.
- Death Ball: a gigantic ball of ki. Hold down fire until charged, and release to fire. This attack is huge in size and does massive damage.

Game Types:

- Death Match: Players start at ~50,000 PL. Additional PL is gained for each kill. On death, the PL of the player resets to the average of all other PLs.
- Tourney: Same rules as DM, but with only two players at a time.
- Team DM: Same rules as DM, but with teams.
- Team LMS: Last Man Standing. If you’ve ever played clan arena you should know this mode. Players start at 1 mil PL, and players who die can not respawn until the next round.
- Oozaru: This game works as regular FFA, except one player is chosen to be the ~50’ tall Oozaru. The player who kills the Oozaru gets to be the Oozaru next. The Oozaru has extra power and speed, and is also a giant compared to the other players.

CVARS:

If you don't already know what a cvar is you should probably ignore this section. These can be used from the q3 console to access features. Most of these features can be set from within the menu.

* g_basepl [1-999]: set the starting power level from one thousand to one mil.
* cg_kitrail [0-99]: set the length of the ki trail. 0 turns it off.
* cg_crosshairhealth [0/1]: turn on or off the feature that makes the crosshair turn red when you score a hit.
* cg_lightauras [0/1]: turn on or off the aura dynamic lights.
* cg_lightexplosions [0/1]: turn on or off the explosion dynamic lights.
* cg_drawKIWarning [0/1]: turn on or off the low ki warning.
* cg_chargeupAlert [0/1]: turn on or off the "ready" message when charging attacks.
* cg_stfu [0/1]: disable character voices when firing attacks.
* cg_lowpolysphere [0/1]: force the use of a lower polycount sphere.
* cg_explosionShell [0/1]: turn on or off the explosion shell.
* cg_drawOwnModel [0/1]: toggle first person between traditional and vis modes.

Help my computer can’t handle BFP!

With all of the graphics options turned on, BFP takes a much beefier system than standard Quake 3. If your computer sucks, don’t despair. We’re looking out for you. BFP is meant to push a p3 500 with 128 megs of memory and a GeForce video card, but it is designed to be able to run playably on a celeron 300 with 128 megs of memory and a TNT2. However, with a much weaker system than that, you are pushing your luck with BFP.

Here are some things you can do to try to boost your performance.

- The first thing to try is turning off dynamic lights in the game options menu. Quake 3 can be slow when it comes to handling dynamic lights, and there are a lot of them in BFP.
- Go into the BFP options menu and use the ki trail slider bar to lower the length of the ki trail. Few systems can handle the ki trail at its max length without framerate loss. With the slider bar all the way to the left, the ki trail will have 0 length. With the slider bar all the way to the right, the ki trail will almost stretch the length of our maps.
- Go under the systems setting menu and lower the resolution to 640x480. Don’t gag, most Quake 3 competition players seem to use this resolution.
- Go into the BFP options menu and set the aura type to “polygonal aura”. This is the least costly aura type. Using the BFP options menu, turn transformation aura off, dynamic aura lights off, dynamic explosion lights off, ki trail off, small own aura on, and ssj perma glow off. With these settings BFP should run about as well as quake 3 does on your computer.
- If you are really having trouble go into the video set-up menu and change the lighting from lightmap to vertex. Everything will look crappy, but the game will run faster.
- If your system is running low on memory, use the BFP options menu to turn Force Default Skins on. This is the BFP equivalent of cg_forceModels in Quake 3. Force Default Skins will disable all custom skins for the models. With this option on, there will only be one skin shared between all Vegetas.

Who made this game, and why is it free?

Bid For Power was made by a group of volunteers. We’re all DBZ fans and most of us have watched all or nearly all the Japanese episodes. I can’t speak for all the reasons of the individual team members, but we all thought the idea of a first person DragonBall Z game was cool enough to want to get involved. It’s free because we are not a company, and don’t have the money to pay off people like Id. This game was created by people, for fun, in their spare time. Everyone on the team contributed to the game design and art style.

- Chris James: project lead, coordinator, original concept, PR.
- Yrgol: programming, game design.
- Rodney: Goku and Piccolo models, animation, additional model tweaking, 2d art, skins.
- Pyrofragger: Gohan, Krillin, and SSJ Goku models, animation, DNamek map, aura and attack models.
- NilreMK: Phase 2 and “secret character” models, animation, attack models, Q3 editing guru.
- Mooky: Vegeta and Freeza models.
- Ansel: lead skin artist, 2d art
- Dethayngel: skins.
- Ebola: skins.
- Anthony: 2d art, char select art.
- Badhead: attack and aura shaders.
- Number17: sounds.
- Yngwei: Snake Way map, skyboxes, performed all music used in BFP.
- GangstaPoodle: GPCity, GPNamek, GPCityDark, and GPCave maps and textures, 2d art.
- Kit Carson: Earth and NamekXL maps and textures.
- $onik: Kami’s Lookout map textures.
- Perfect Chaos: additional map textures.
- Nat: webmaster, additional programming.
- Disco Stu: web design


 

# zanzoken - 12/17/2000 by Yrgol	

Zanzoken is in now. I'm doing it as a short range controlled-direction teleport. Double tap right or left in combination with other movement in order to zanzoken. I don't want to get into specifics but yes there are restrictions and costs to prevent this from being used all the time.

 

# stuff - 12/14/2000 by Yrgol	

As you can tell I'm still making the mod. The bots own now. If all that mattered were code, bfp would be ready to release. Everything that I said was going to be in phase 1 is in there now and then some, and the bugs are minimal. A few of our models still need some tweaking before the whole is releasable.

Funimation did indeed send us a cease and decist letter within days of selling dbz game rights to Infogrames. However, they have decided to take a closer look at the situation. We did not want to jump the gun on being foxed, because things are still fairly up in the air. We did not want the slashdot post to happen.

Bid For Power will be released. It will most likely have Dragon Ball Z characters in it when it is released.

17-3 (not finished):
* made bots aim beams.
* made bots charge attacks randomly above the minimum.
* made bots melee.
* made bots act blind when hit with the taiyoken.
* made bots aware of splash damage on charge-up attacks.
* lowered shyogeki ha damage from 20 to 5.
* lowered shyogeki ha range from 8192 to 1200.
* lowered shyogeki ha spread.
* made homing attacks easier to dodge.
* lowered kiaiho range from 8192 to 1000.
* made kiaiho do a stable 20% damage.
* removed the damage boost for ki boosting.
* got rid of the character change messages because they never worked right.
* shrunk the pre-fire deathball a bit so it doesn't clip freeza's head.
* made blindness not last through death.
* got rid of the melee axehandle smash direction.
* fixed a bug in melee that caused a crash.
* lowered hit-stun time from 3 seconds to 2.
* capped self-splash at 20%.
* added in the separate kienzan attack shader.
* shrunk freeza's kienzan.
* fixed a bug that made the kienzans stay around far longer than they should.
* fixed a bug that made kienzans hit way too fast and too often.
* gave the kienzans a max number of hits, which fixes a memory leak.
* fixed a bug with the animation timers.
* fixed a bug with switching weapons while firing a beam.
* fixed a bug with switching weapons while charging up an attack.
* made charging only prevent intentional movement, not glue you in place.

17-2:
* made bots be 100% agressive all the time.
* made bots fly a better.
* taught bots how to use charge-up attacks and beams.
* fixed bot aim.
* upped the charge time for the chobakuretsumaha and sokidan.
* scaled down the pre-fire flashes of most attacks.
* fixed the spelling and grammer in the bot files and fixed the error messages.

17-1:
* redid third person view.
* got around the gimble lock for flight so you can go upside down.
* changed default cg_thirdpersonheight to -50.


 

# more sutff than you wanted - 11/29/2000 by Yrgol	

16-3/17:
* fixed the timing of the "ready" message with the kamehameha.
* made a bfp.cfg file so that bfp will work fine by using the mods menu.
* fixed a bug that caused untextured weapon flashes to appear.
* fixed a bug that caused attack voices to sometimes come out at the wrong time.
* added cg_stfu to turn the voices off.
* fixed a bug that made melee mess up charge-up attacks.
* fixed a bug that caused a lot of problems in the char select screen.
* fixed a bug that made melee screw up the weapon time and hit way too fast.
* fixed a bug that made the charge-o-meter misread the sokidan/chobakuretsumaha power.
* fixed another bug with the ready message.
* added cull disable to all the explosion shaders so they don't disappear when you get too close.
* added an explosion shell/shockwave. can be turned off with cg_explosionShell 0.
* added cg_lowpolysphere to toggle between a low and a high polycount sphere model.
* fixed the infamous death-spawn bug.
* fixed the sizing of the accept button in the video setup menu.
* seemingly accidentally fixed the problem of ki trails showing through walls.
* made cg_thirdpersonrange and cg_thirdpersonheight get saved in bfp.cfg.
* gave cg_thirdpersonheight a default of 20.
* increased freezabeam explosion radius.
16-2:
* added g_basePL with a slider in the server startup to define starting PL.
* gave the renzoku an alternating firing offset.
* cut chobakuretsumaha damage in half.
* upped the ki cost of the chobakuretsumaha.
* lowered freezabeam damage.
* made melee hit stun not happen when both people are holding melee.
* made homing missiles select a target based on angle instead of distance.
* refined homing angle selection somewhat.
* fixed the sbc spiral so it disappears less often.
* fixed jump (detection and animation).
* made the kienzan stop disappearing when it hits other missiles.
* added self splash damage back in.
* made blocking deflect missiles.
* added in 17's explosion sounds.
* added in character sounds for sbc, kamehameha, final flash, bb attack, masenko.
* hopefully fixed the ride me big boy bug.

16-1:
* made the spiral not disappear when two people sbc at the same time
* made ki trails a little more robust
* forced r_railCoreWidth to 15 on startup.
* removed a bug with the ssjhead getting an aura when it shouldn't.
* fixed a bug that caused the char select screen to crash the game.
* resized images so they were powers of 2.
* got rid of "unknown gametype" when connecting to a oozaru or lms server.
* homing attacks don't turn all the way around anymore.
* homing attacks no longer lose their velocity when homing.
* tweaked the various homing missile speeds to account for the new code.
* made taiyoken based on FOV, meaning you have to see it to go blind.
* set the size of menu buttons to 80x80 and made those values
settable through one file instead of 30 different files.
* made the eyebeam visible from all perspectives.
* upped the eyebeam damage.
* made the eyebeam not draw when holding melee.
* fixed a rotation problem on the auras.
* fixed a problem with showing the wrong gametype name when starting a server.

16:
* decreased melee dive range from 2000 to 700.
* hopefully fixed the problem of melee dive teleporting you into objects.
* new menu.
* new attack icons.
* new hud.
* new ki trail.
* fixed the servers menu to differentiate between bfp gametypes.
* removed the "impressive" for the freezabeam.
* lowered the ki cost of the freezabeam from 1000 to 500.
* made the freezabeam explode when it hits a player instead of going through them.
* gave the freezabeam explosion some damage.
* changed the sound played for the freezabeam.
* fixed a bug in the eyebeam that made the beam not show up.
* changed the name of the sbc to makankosappo.
* removed drowning.
* made the speed boost depend on PL instead of power tier.
* made cg_kitrail be a slider var from 0 to 99 for ki trail length.

15-8:
* removed heals from spawning on maps.
* increased visual explosion size.
* made charging a lot more effective.
* cut damage in half.
* made melee dive a lot more solid.
* gave the melee hit sound an origin.

15-7:
* made the chargeup alert display in third person
* moved the chargeup alert to the bottom of the screen.
* plays the pain sounds based on percent of max health.
* upped masenko damage.
* made the hud player counter not include dead people.
* removed gibs for killing someone. you can still gib a corpse.
* made the prepare anim play for kakusan.
* fixed the timing on at least some charge up attack anims.
* added a flash model to firing beams.
* added KamehamehaFlashShader etc for the beam attacks.
this shader needs to be opaque enough to hide the beam origin
* scaled down the pre-fire ball for beam attacks.
* removed the standard flash entirely.

15-6:
* added a "ready to fire" message for charge-up attacks.
* added cg_chargeupAlert to turn off that message.
* fixed a bug that would let attacks get kienzan properties.
* upped kakusan damage some.
* lowered taiyoken damage to 1%.
* added a visibility check on taiyoken. only checks barriers, not direction.
* increased taiyoken chargeup time.
* shortened blindness time some.
* made the melee_axehandle animation run, but it's not very noticable.
* toned down the angry gohan.
* made the prepare anim play for the angry gohan.
* changed the scoreboard name of the angry gohan to the Angry Gohan Attack.
* fixed "the incredible eternal kienzan" bug.

15-5:
* first person vis mode works. it works best with models that have tag_eyes (gohan/krillin).
* angry gohan no longer draws while meleeing or charging.
* took out the announcer sounds for player names.
* blindness no longer stays with you through death.
* took out the ring model for explosions on players.
* fixed a scaling problem in the explosion ring that made it go black.
* removed the eject brass option from the game options menu.
* individual score now survives death in LMS mode.
* following in LMS spectator mode no longer fucks the next round.
* got rid of some red in the menu.
* gave explosions and missiles dynamic lights.
* added cg_lightExplosions to turn off dynamic explosion lights.
* removed the chargeup anim for taiyoken.

15-4: lost all code since 15, had to redo stuff.
--SHADERS--
* added shaders for kamehameha and masenko.
* added separate beam shaders.
* added separate shaders for explosions.
* added a shader for the freezabeam explosion
--WEAPONS--
* freeza's second attack no longer ejects brass.
* fixed manual beam detonation.
* missiles should collide with each other a bit better.
* fixed homing attack collisions.
* increased eye beam range.
* removed self-damage.
* gave deathball a min charge up time (it was supposed to have one).
* gave masenko a min charge up time (it was supposed to have one).
* decreased the chargeup speed of sokidan/chobakuretsumaha.
* made all weapons explode when they hit a player.
* put the BIG BANG in the BIG BANG attack
* pumped up the deathball.
* can't continue firing a beam while charging, blocking,
melee fighting, or in hit-stun.
* beam attacks use the sphere as the pre-fire flash.
* beams now trump missiles for mid-air collisions.
* lowered renzoku damage.
* made taiyoken work.
* gave the SBC a spiral trail.
* weapon firing sound plays while firing beams instead of
while charging them.
* made the freeza beam red and gave it an explosion.
* gave explosions a smoke ring.
--OTHER--
* custom skins now work fine. if you use a custom
vegeta skin that I don't have, instead of showing
up as sarge you will show up as vegeta/default on
my screen.
* somewhat improved bots.
* cg_forceModel removed.
* added cg_forceSkin to force the default skin for each model.
* changed obituary messages.
* beam attacks no longer blind you in first person view.
* switched from the nuke model to the sphere model for explosions.
* tweaked explosions a lot.
* picking up a normal health after a mega health won't set
you back to 100% health anymore.
* no longer plays the melee sound for missiles.
* unfucked spectator mode.
* switching chars while in LMS mode causes a suicide like in
any other game mode.
