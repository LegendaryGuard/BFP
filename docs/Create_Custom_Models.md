# Creating custom plugin models for Bid For Power

This document assumes an understanding of how to create models for Quake
3 Arena.

## Attacksets

All BFP models must use one of the attacksets defined in the file
*bfp_attacksets.cfg*. Attacksets are assigned to models by comparing
the prefix of the model to the entries for \"modelPrefix\" in
*bfp_attacksets.cfg*. If there is a match, the model is assigned the
attacks for the matching attackset.

In order to create a model named \"bob\" that uses the first attackset, bob must exist in the directory \"models/players/bfp1-bob\".

## Animations

All BFP Models must have the following animations in the following
order.

```
BOTH_DEATH1
BOTH_DEAD1

BOTH_DEATH2
BOTH_DEAD2

BOTH_DEATH3
BOTH_DEAD3


TORSO_GESTURE

TORSO_STAND

TORSO_RUN

TORSO_BLOCK

TORSO_STUN

TORSO_FLYA
TORSO_FLYB

TORSO_CHARGE

TORSO_MELEE_READY
TORSO_MELEE
TORSO_MELEE_STRIKE
TORSO_MELEE_AXEHANDLE

LEGS_WALKCR
LEGS_WALK

LEGS_RUN

LEGS_BACK

LEGS_SWIM

LEGS_JUMP
LEGS_JUMPB

LEGS_IDLE
LEGS_IDLECR

LEGS_TURN

LEGS_FLYIDLE
LEGS_FLYA
LEGS_FLYB

LEGS_CHARGE

LEGS_MELEE
LEGS_MELEE_STRIKE

TORSO_ATTACK0_PREPARE
TORSO_ATTACK0_STRIKE

TORSO_ATTACK1_PREPARE
TORSO_ATTACK1_STRIKE

TORSO_ATTACK2_PREPARE
TORSO_ATTACK2_STRIKE

TORSO_ATTACK3_PREPARE
TORSO_ATTACK3_STRIKE

TORSO_ATTACK4_PREPARE
TORSO_ATTACK4_STRIKE
```

## Tags

All bfp models are required to have a tag named \"tag_eyes\" on the
model\' head. This tag should be appropriate for placing a viewpoint.

Models should make use of additional tags placed on the torso in order
to position attacks. The use of these tags is defined by the skin
config file.

## Transformations

Each model is allowed to have 1 transformation which is shared by all
skins for that model. This transformation happens when the player
reaches the top power tier. If the files ssjhead.md3 and ssjhead.skin
exist, the head will be swapped for the transformed head. If the
files ssjtorso.md3 and ssjtorso.skin exist, those files will be used for
the transformed torso. Likewise if the files ssjlegs.md3 and
ssjlegs.skin exist, they will define how the transformed legs will look.

## The Skin Config File

The skin config file can be used to define attack names, how attacks are positioned on the model, how the attacks look, and how the attacks sound. Each of the default BFP models has a file called \"default.cfg\" in the same directory as the model. This file provides the default values for all models of the same attackset. Skin config files for a specific non default skin can be created by naming the file [skinname].cfg and placing it in the same directory as the **.md3** files.

When a custom model \"bfp1-bob\" using the skin \"red\" is loaded, the
skin config file for the \"bfp1-kyah/default\" is read in to the game. Then, the skin config for \"bfp1-bob/default\" is loaded, overriding
values from the default model. Finally, the skin config for
\"bfp1-bob/red\" is loaded.

With the exception of the default models, all of the following skin
config options are optional. Attack index is a number 0-4, and is
used to identify which attack the setting applies to. In order to
turn off a value set by a default model, pass in 0 as the argument.
<br/><br/>

- ```attackName [attack index] ["name of attack"]```

The name of the attack, used for display when selecting an attack.
<br/><br/>

- ```attackIcon [attack index] ["path of attack icon shader"]```

The icon used by this attack during weapon selection. Only the
default icons are displayed in the char select screen.
<br/><br/>

- ```attackTag [attack index] ["name of tag"]```

The string value passed in is used to position the flash and other
weapon effects. *attackTag 0 \"tag_right\"*.
<br/><br/>

- ```attackTagPart [attack index] ["head" / "torso"]```

attackTagPart is the body part on which the attack tag exists.   
*attackTag 0 \"torso\"*.
<br/><br/>

- ```constantFireAttack [attack index] [0/1]```

This defines whether the attack is treated as a constant fire attack.   
It is used to have the flash sound only play once when fire is held
down.
<br/><br/>

- ```lightningBolt [attack index] [0/1]```

*lightningBolt 0 1* makes the first attack act visibly like the
lightning gun. The beam shader is used for the lightning rod.   
constantFireAttack should be set for any attack that uses lightningBolt.
<br/><br/>

- ```noExplosion [attack index] [0/1]```

Used to cause the attack to have no visible or audible explosion of any
kind.
<br/><br/>

- ```noExplosionSound [attack index] [0/1]```

Used to prevent the explosion sounds from playing for this attack.
<br/><br/>

- ```attackFireVoice [attack index] ["path of sound"]```

A voice sound played once every time the attack is fired.
<br/><br/>

- ```attackChargeVoice [attack index] [charge count] ["path of sound"]```

The charge count must be a number 0-4. *attackChargeVoice 0 4 \"sound\\bfp\\blah.wav\"* would play the sound once when the attack is
charged to the 4th dot.
<br/><br/>

- ```missileSound [attack index] ["path of sound"]```

A looping sound played at the origin of the missile. This has no
effect for hitscan attacks.
<br/><br/>

- ```chargeSound [attack index] ["path of sound"]```

A looping sound played while charging an attack. Only has effect for
charge attacks.
<br/><br/>

- ```flashSound [attack index] ["path of sound"]```

A sound played once when the attack is fired.
<br/><br/>

- ```firingSound [attack index] ["path of sound"]```

A looping sound played while firing the attack. For attacks with
constantFireAttack set, the firing sound will be played while fire is
held down. For beam attacks, the firing sound will loop while the
beam exists. For charge autofire attacks, this will be played once
the attack starts and until fire is released.
<br/><br/>

- ```missileDlight [attack index] [int]```

The radius of the dynamic light for this attack. Set this to 0 for no dynamic light.
<br/><br/>

- ```missileDlightColor [attack index] [red float] [green float] [blue float]```

The color of the dynamic light. *missileDlightColor 0 1.0 1.0 1.0* is max intensity for attack 0.
<br/><br/>

- ```missileTrailFunc [attack index] ["beam" / "rocket" / "spiralbeam" / "none"]```

The "beam" argument is used for regular beam attacks.   
"spiralbeam" uses the spiral beam trail, which does not bend.   
"rocket" is a trail of smoke puffs. "none" is default.
<br/><br/>

- ```missileTrailTime [attack index] [int]```

Only used for rocket trails. The length of time that each smoke puff lives.
<br/><br/>

- ```missileTrailRadius [attack index] [int]```

Only used for rocket trails. The radius of each smoke puff.
<br/><br/>

- ```beamShader [attack index] ["name of shader"]```

The shader used for the beam part of beam attacks and the rod part of lightning attacks.   

Example: `beamShader 4 "gfx/weaponfx/shader"`
<br/><br/>

- ```spiralBeamShader [attack index] ["name of shader"]```

Only has effect for attacks that use the spiralbeam trail. This shader is used for the spiral part of the beam.
<br/><br/>

- ```flashModel [attack index] ["name of model"]```

The model used for the weapon flash. If this is not set or set to 0 and flashShader is set, the flash shader will be treated as a sprite.
<br/><br/>

- ```flashShader [attack index] ["name of shader"]```

The shader used for the flash.
<br/><br/>

- ```flashRadius [attack index] [int]```

Only used for sprite flashes. `flashRadius` is the radius of the flash sprite.
<br/><br/>

- ```flashScaleFactor [attack index] [float]```

The scale factor used for the flash. Used to resize non-sprite flashes.
<br/><br/>

- ```firingFlashRadius [attack index] [int]```

The radius of the firing flash. This is used for beam attacks while the beam is firing.
<br/><br/>

- ```firingFlashScaleFactor [attack index] [float]```

The scale factor used for the firing flash. Used to resize non-sprite flashes.
<br/><br/>

- ```missileShader [attack index] ["name of shader"]```

The shader used for the missile. Also used for the tip of beam
attacks. Has no effect for hitscan attacks.
<br/><br/>

- ```missileModel [attack index] ["name of model"]```

The model used for the missile. Also used for the tip of beam
attacks. If this is not set or set to 0, the missile shader will be
treated as a sprite. Has no effect for hitscan attacks.
<br/><br/>

- ```missileRotation [attack index] [int]```

The rotation of the missile.
<br/><br/>

- ```missileSpinHoriz [attack index] [0/1]```

Used to switch the axis of rotation on the missile.
<br/><br/>

- ```missileRadius [attack index] [float]```

Used to scale the size of sprite missiles. The actual scale value is `missileRadius + missileRadius ChargeMult * (number of points charged over the min)`
<br/><br/>

- ```missileRadiusChargeMult [attack index] [int]```

See missileRadius.
<br/><br/>

- ```missileScaleFactor [attack index] [float]```

Used to scale the size of the missile. The actual scale value is `missileScaleFactor + missileScaleFactorChargeMult * (number of points charged over the min)`
<br/><br/>

- ```missileScaleFactorChargeMult [attack index] [float]```

See missileScaleFactor.
<br/><br/>

- ```explosionModel [attack index] ["name of model"]```

The model used for the central sphere of the explosion.
<br/><br/>

- ```explosionShader [attack index] ["name of shader"]```

The shader used for the central sphere of the explosion.
<br/><br/>

- ```explosionRing [attack index] [0/1]```

Used to turn on or off the ring model part of the explosion.
<br/><br/>

- ```explosionShell [attack index] [0/1]```

Used to turn off the outer sphere of the explosion.
<br/><br/>

- ```explosionRocks [attack index] [int]```

The number of rock particles created by the explosion divided by 2.
<br/><br/>

- ```explosionSparks [attack index] [int]```

The number of sparks created by the explosion divided by 3.
<br/><br/>

- ```explosionSmoke [attack index] [int]```

The number of smoke particles created by the explosion. More than 3 tends to create unacceptable slowdowns.
<br/><br/>

- ```explosionSmokeRadius [attack index] [int]```

The radius of each smoke particle.
<br/><br/>

- ```explosionSmokeLife [attack index] [int]```

The length of time the smoke particle lives.
<br/><br/>

- ```explosionSmokeSpeed [attack index] [int]```

The initial speed of the explosion smoke.
<br/><br/>

- ```explosionScaleFactor [attack index] [float]```

Used to scale the central explosion. The actual scale value is `explosionScaleFactor + explosionScaleFactorChargeMult * (number of points charged over the min)`
<br/><br/>

- ```explosionScaleFactorChargeMult [attack index] [float]```

See explosionScaleFactor.
<br/><br/>

- ```explosionRingScaleFactor [attack index] [float]```

Used to scale the ring model part of the explosion. The actual scale value is `explosionRingScaleFactor + explosionRingScaleFactorChargeMult * (number of points charged over the min)`
<br/><br/>

- ```explosionRingScaleFactorChargeMult [attack index] [float]```

See explosionRingScaleFactor
<br/><br/>

- ```explosionShellScaleFactor [attack index] [float]```

Used to scale the outer sphere of the explosion. The actual scale
value is `explosionShellScaleFactor + explosionShellScaleFactorChargeMult * (number of points charged over the min)`
<br/><br/>

- ```explosionShellScaleFactorChargeMult [attack index] [float]```

See explosionShellScaleFactor
<br/><br/>

- ```end```

Should be the last word in the skin cfg file.
