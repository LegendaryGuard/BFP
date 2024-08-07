# BFP CVARS

## PENDING:

- g_basepl [1-999]: set the starting power level from one thousand to one mil.
- g_kiChargePct = "15"
- g_kiCharge = "0"
- g_kiRegenPct = "0.6"
- g_kiRegen = "0"
- g_boostCostPct = "0"
- g_boostCost = "150"
- g_flightCostPct = "0"
- g_flightCost = "50"
- g_chargeDelay = "250"

#### Cvar Gametypes:

- g_gametype 3 = sur (survival)
- g_gametype 4 = ooz (oozaru, in original BFP, that hasn't been added on UI, but add it anyway)
- g_gametype 5 = tdm (team deathmatch) // originally, in Q3, this number is 3
- g_gametype 6 = lms (last man standing)
- g_gametype 7 = ctf (capture the flag) // originally, in Q3, this number is 4

... and maybe more, look in [cvar_bfp_list.txt](docs/cvar_bfp_list.txt)


## WIP:


#### Cvar Gametypes:

- g_gametype 2 = single player (in original BFP, that doesn't show in UI, so make Single Player option hidden)

## DONE BUT NOT IN CGAME:

- cg_stfu [0/1]: disable character voices when firing attacks.
- cg_lowpolysphere [0/1]: force the use of a lower polycount sphere.
- cg_lightexplosions [0/1]: turn on or off the explosion dynamic lights.
- cg_chargeupAlert [0/1]: turn on or off the "ready" message when charging attacks.
- cg_explosionShell [0/1]: turn on or off the explosion shell.
- cg_explosionSmoke [0/1]: turn on or off the explosion smoke.
- cg_explosionRing [0/1]: turn on or off the explosion ring.
- cg_particles [0/1]: turn on or off particle effects.


## COMPLETED:

- [x] ~~cg_superdeformed~~
- [x] ~~cg_kiTrail~~
- [x] ~~cg_yrgolroxor~~
- [x] ~~cg_thirdPersonHeight~~
- [x] ~~cg_fixedThirdPerson~~
- [x] ~~cg_drawOwnModel~~
- [x] ~~cg_stableCrosshair~~
- [x] ~~cg_musicUnpacked~~
- [x] ~~cg_crosshairColor~~
- [x] ~~cg_crosshairHealth~~
- [x] ~~cg_flytilt~~
- [x] ~~cg_drawKiWarning~~
- [x] ~~cg_lightAuras~~
- [x] ~~cg_smallOwnAura~~
- [x] ~~cg_lightweightAuras~~
- [x] ~~cg_polygonAura~~
- [x] ~~cg_highPolyAura~~
- [x] ~~cg_playHitSound~~
- [x] ~~g_noFlight (disables "fly" bind too, original BFP has a leak though)~~
- [x] ~~g_blockLength~~
- [x] ~~g_blockDelay~~
- [x] ~~g_blockCost~~
- [x] ~~g_blockCostPct~~
- [x] ~~g_hitStun~~
- [x] ~~g_meleeOnly~~
- [x] ~~g_meleeRange~~
- [x] ~~g_meleeDiveRange~~
- [x] ~~g_meleeDamage~~

#### Cvar Gametypes:

- [x] ~~g_gametype 0 = dm (aka FFA)~~ (already in Q3 code)
- [x] ~~g_gametype 1 = 1v1 (tournament)~~ (already in Q3 code)