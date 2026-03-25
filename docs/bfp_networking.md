- The following stuff is used in original BFP networking and has been found using engine code. Although optimizations or ways to not use network flags or values ​​are found, the original flags and indexes must be present to see how it was envisioned.

    * PMF_ flags: the same as Quake 3
    * EF_ flags: the same as Quake 3
    * STAT_ indexes:
        * STAT_0 = health
        * STAT_3 = armor
        * STAT_4 = dead yaw
        * STAT_5 = bit mask for client ready intermission
        * STAT_6 = powerlevel
        * STAT_7 = flight jump anim transition seconds, the maximum is until 21 sec and stops changing to 0, even when stop flying also reproduces this stat index like starting to fly (looks weird)
        * STAT_8 = ki
        * STAT_9 = maximum ki
        * STAT_10 = melee attack time
        * STAT_13 = looks like it's a beam firing state
        * STAT_14 = force field weapon state
        * STAT_15 = fly tilt: moving left is lesser than 0 until -80, moving right is more than 0 until 80
    * PERS_ (persistant) indexes:
        * PERS_0 = score
        * PERS_1 = total points damage inflicted so damage beeps can sound on change
        * PERS_2 = rank
        * PERS_3 = player team
        * PERS_4 = spawn count (respawn)
        * PERS_7 = health/armor of last person we attacked
        * PERS_8 = count of the number of times you died
        * PERS_11 = defend awards (defending the flag in CTF)
        * PERS_13 = ??? (Q3: kills with the guantlet)
        * PERS_14 = powerlevel
        * PERS_15 = ??? (it appears when spawning at the first time of all in-game)
    * PW_ (powerups) indexes:
        * PW_0 = that's where PW_HASTE and PW_BATTLESUIT are marked as PW_NONE after picking up
        * PW_1 = PW_QUAD
        * PW_2 = PW_REDFLAG placed on PW_BATTLESUIT
        * PW_3 = PW_BLUEFLAG placed on PW_HASTE
        * PW_5 = PW_FLIGHT reused for flight enabled/disabled
        * PW_6 = ki recharge
        * PW_7 = ki use/toggle
        * PW_8 = blocking seconds (to defend yourself from melee, ki attacks: beams, projectiles, ...)
        * PW_9 = melee toggle
        * PW_10 = hit stun
        * PW_11 = ki attack charge points
        * PW_12 = monster flag (g_gametype 4)
        * PW_13 = beam firing state?
        * PW_14 = jump (what the hell?)
    * WP_ (weapons) indexes:
        * WP_0, WP_1, WP_2, WP_3 and WP_4 are the weapons in the attack selection
        * WP_4 (WP_GRENADE_LAUNCHER) = as the last ki attack, but also when the player is being attacked or damaged, begins from 2000 as milliseconds until 0, looks like a timer, the purpose remains unknown
        * WP_5 (WP_ROCKET_LAUNCHER) = ki recharge time to enable
        * WP_6 (WP_LIGHTNING) = hit stun delay (after receiving a hit stun)
        * WP_7 (WP_RAILGUN) = block delay (after using block)
        * WP_8 (WP_PLASMAGUN) = ki use/boost toogle
        * WP_9 (WP_BFG) = flight toggle key control (to avoid spamming the enable flight key, just enable once and disable once)
        * WP_10 (WP_GRAPPLING_HOOK) = blind seconds
        * WP_11 = for rapid ki attacks like ki storm (alternates: -1 and 1)
        * WP_13 = toggles if the player can use zanzoken or not
        * WP_14 = directional left and right keys to move left or right while pressing, adds time msec, not sure if that's a timer to handle WP_13 for zanzoken
        * WP_15 = enables beam struggle

    * EV_* (events) indexes:
```c
        * EV_NONE                 // 0
        * EV_UNUSED_INDEX1        // 1
        * EV_UNUSED_INDEX2        // 2
        * EV_UNUSED_INDEX3        // 3
        * EV_UNUSED_INDEX4        // 4
        * EV_UNUSED_INDEX5        // 5
        * EV_UNUSED_INDEX6        // 6
        * EV_UNUSED_INDEX7        // 7
        * EV_UNUSED_INDEX8        // 8
        * EV_UNUSED_INDEX9        // 9
        * EV_MELEE_READY          // 10, preparing melee
        * EV_MELEE                // 11, melee attack
        * EV_UNUSED_INDEX12       // 12
        * EV_TIER_RESET           // 13, reset tier when the player respawns and changes to the default or a bit less ki energy?
        * EV_TIER_0               // 14, EV_TIER_0-4 (14-18), when the player frags, increases their PL and obtains a new skill (in the last tier, transforms)
        * EV_TIER_1               // 15
        * EV_TIER_2               // 16
        * EV_TIER_3               // 17
        * EV_TIER_4               // 18
        * EV_ZANZOKEN_IN          // 19, Short-Range Teleport (Zanzoken)
        * EV_ZANZOKEN_OUT         // 20, stop/leaves zanzoken
        * EV_KI_BOOST             // 21, enables aura/ki trail
        * EV_ENABLE_FLIGHT        // 22, enable flight
        * EV_FOOTSTEP             // 23
        * EV_FOOTSTEP_METAL       // 24
        * EV_FOOTSPLASH           // 25
        * EV_FOOTWADE             // 26
        * EV_SWIM                 // 27
        * EV_STEP_4               // 28
        * EV_STEP_8               // 29
        * EV_STEP_12              // 30
        * EV_STEP_16              // 31
        * EV_FALL_SHORT           // 32
        * EV_FALL_MEDIUM          // 33
        * EV_FALL_FAR             // 34
        * EV_JUMP_PAD             // 35
        * EV_JUMP                 // 36
        * EV_JUMP_2               // 37
        * EV_WATER_TOUCH          // 38
        * EV_WATER_LEAVE          // 39
        * EV_WATER_UNDER          // 40
        * EV_WATER_CLEAR          // 41
        * EV_ITEM_PICKUP          // 42
        * EV_GLOBAL_ITEM_PICKUP   // 43
        * EV_NOAMMO               // 44
        * EV_CHANGE_WEAPON        // 45
        * EV_FIRE_WEAPON          // 46
        * EV_USE_ITEM0            // 47
        * EV_USE_ITEM1            // 48
        * EV_USE_ITEM2            // 49
        * EV_USE_ITEM3            // 50
        * EV_USE_ITEM4            // 51
        * EV_USE_ITEM5            // 52
        * EV_USE_ITEM6            // 53
        * EV_USE_ITEM7            // 54
        * EV_USE_ITEM8            // 55
        * EV_USE_ITEM9            // 56
        * EV_USE_ITEM10           // 57
        * EV_USE_ITEM11           // 58
        * EV_USE_ITEM12           // 59
        * EV_USE_ITEM13           // 60
        * EV_USE_ITEM14           // 61
        * EV_USE_ITEM15           // 62
        * EV_ITEM_RESPAWN         // 63
        * EV_ITEM_POP             // 64
        * EV_PLAYER_TELEPORT_IN   // 65
        * EV_PLAYER_TELEPORT_OUT  // 66
        * EV_GRENADE_BOUNCE       // 67
        * EV_GENERAL_SOUND        // 68
        * EV_GLOBAL_SOUND         // 69
        * EV_GLOBAL_TEAM_SOUND    // 70
        * EV_BULLET_HIT_FLESH     // 71
        * EV_BULLET_HIT_WALL      // 72
        * EV_MISSILE_HIT          // 73
        * EV_MISSILE_MISS         // 74
        * EV_MISSILE_MISS_METAL   // 75
        * EV_MISSILE_DETONATE     // 76, in some moment, ki attack beam/projectile
        * EV_RAILTRAIL            // 77
        * EV_SHOTGUN              // 78
        * EV_UNUSED_INDEX79       // 79
        * EV_PAIN                 // 80
        * EV_DEATH1               // 81
        * EV_DEATH2               // 82
        * EV_DEATH3               // 83
        * EV_OBITUARY             // 84
        * EV_POWERUP_QUAD         // 85
        * EV_POWERUP_BATTLESUIT   // 86
        * EV_UNUSED_INDEX87       // 87
        * EV_GIB_PLAYER           // 88
        * EV_UNUSED_INDEX89       // 89
        * EV_UNUSED_INDEX90       // 90
        * EV_SCOREPLUM            // 91
        * EV_TAUNT                // 92
        * EV_UNUSED_INDEX93       // 93
        * EV_UNUSED_INDEX94       // 94
        * EV_UNUSED_INDEX95       // 95
        * EV_UNUSED_INDEX96       // 96
        * EV_DEBUG_LINE           // 97
        * EV_STOPLOOPINGSOUND     // 98
        * EV_BEAM_STRUGGLE        // 99, beam struggle
```

    * bg_itemlist indexes:
```c
        * NULL                   // 0
        * item_armor_shard       // 1
        * item_armor_combat      // 2
        * item_armor_body        // 3
        * item_health_small      // 4
        * item_health            // 5
        * item_health_large      // 6
        * item_health_mega       // 7
        * weapon                 // 8
        * weapon                 // 9
        * weapon                 // 10
        * weapon                 // 11
        * weapon                 // 12
        * NULL                   // from 13 until 116
        * holdable_teleporter    // 117
        * holdable_medkit        // 118
        * item_quad              // 119
        * item_enviro            // 120
        * item_haste             // 121
        * item_invis             // 122
        * team_CTF_redflag       // 123
        * team_CTF_blueflag      // 124
```

In original BFP, when the players aim with the crosshair against their opponents to show their powerlevel, uses `entityState_t->time2` (32-bit integer size) instead `entityState_t->frame` (16-bit integer size). 
With `entityState_t->frame` was enough for that size from 0 until 1000.
