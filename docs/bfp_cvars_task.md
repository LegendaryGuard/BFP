# BFP CVARS

## PENDING:

- g_basepl [1-999]: set the starting power level from one thousand to one mil.
- cg_kitrail [0-99]: set the length of the ki trail. 0 turns it off.
- cg_lightauras [0/1]: turn on or off the aura dynamic lights.
- cg_lightexplosions [0/1]: turn on or off the explosion dynamic lights.
- cg_chargeupAlert [0/1]: turn on or off the "ready" message when charging attacks.
- cg_stfu [0/1]: disable character voices when firing attacks.
- cg_lowpolysphere [0/1]: force the use of a lower polycount sphere.
- cg_explosionShell [0/1]: turn on or off the explosion shell.
- cg_explosionSmoke [0/1]: turn on or off the explosion smoke.
- cg_explosionRing [0/1]: turn on or off the explosion ring.
- cg_particles [0/1]: turn on or off particle effects.
- cg_playHitSound [0/1]: turn on or off the q3 hit "ping".
- cg_flytilt [0/1]: toggle the tilting during flight.
- g_blockLength = "3"
- g_blockDelay = "2"
- g_kiChargePct = "15"
- g_kiCharge = "0"
- g_kiRegenPct = "0.6"
- g_kiRegen = "0"
- g_blockCostPct = "3"
- g_blockCost = "0"
- g_boostCostPct = "0"
- g_boostCost = "150"
- g_flightCostPct = "0"
- g_flightCost = "50"
- g_chargeDelay = "250"
- g_meleeRange = "32"
- g_meleeDiveRange = "700"
- g_meleeDamage = "10"

#### Cvar Gametypes:

- g_gametype 3 = sur (survival)
- g_gametype 4 = ooz (oozaru, in original BFP, that hasn't been added on UI, but add it anyway)
- g_gametype 5 = tdm (team deathmatch) // originally, in Q3, this number is 3
- g_gametype 6 = lms (last man standing)
- g_gametype 7 = ctf (capture the flag) // originally, in Q3, this number is 4

... and maybe more, look in [cvar_bfp_list.txt](docs/cvar_bfp_list.txt)


## WIP:

- g_hitStun [0/1]: turn on or off the melee hit stun.
- cg_drawKiWarning [0/1]: turn on or off the low ki warning.
- cg_crosshairhealth [0/1]: turn on or off the feature that makes the crosshair turn red when you score a hit.

#### Cvar Gametypes:

- g_gametype 2 = single player (in original BFP, that doesn't show in UI, so make Single Player option hidden)


## DONE BUT NOT IN UI SETTINGS:

(TODO: Add in BFP Options -> Fixed Third Person - radio button )
- cg_fixedThirdPerson

(TODO: Add in BFP Options -> Viewpoint: (Third Person (cg_thirdPerson 1) / First Person (cg_thirdPerson 0 and cg_drawOwnModel 0) / First Person Vis Mode (cg_thirdPerson 0 and cg_drawOwnModel 1) )
- cg_drawOwnModel

(TODO: Add in BFP Options -> Accurate Crosshair - radio button )
- cg_stableCrosshair [0/1]: turn off the accurate crosshair

(TODO: Add in BFP Options -> Disable voices: )

(TODO: Add in BFP Options -> Low Polycount Sphere: )

... look in the BFP Options menu screenshot in [README.md](README.md)


## COMPLETED:

- [x] ~~cg_yrgolroxor~~
- [x] ~~cg_thirdPersonHeight~~

#### Cvar Gametypes:

- [x] ~~g_gametype 0 = dm (aka FFA)~~ (already in Q3 code)
- [x] ~~g_gametype 1 = 1v1 (tournament)~~ (already in Q3 code)