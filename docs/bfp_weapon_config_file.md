# Weapon Config File

The weapon config file (*bfp_weapon.cfg* and *bfp_weapon2.cfg*) is used to define attacks, every attack has its own speed, radius, damage, ... 
Reads *bfp_weapon.cfg*, and after *bfp_weapon2.cfg* (if exists), once in the server at the start. There are some properties used in `bg_pmove.c`.

### (attack_name)

- `(attack_name)`<br/>
Attack tag, starts to read the attack properties below and continues setting the properties of this attack until the next attack tag or the [end](#end) of file.

### weaponNum

- ```weaponNum [int]```<br/>
Attack identifier.

### attackType

- ```attackType [missile/rdmissile/beam/sbeam/hitscan/forcefield]```<br/>
Sets attack type. 
  - `missile` shots a projectile.
  - `rdmissile` is like a projectile that splits projectiles by the number of ki charge points. Requires another attack to set, see [explosionSpawn](#explosionSpawn).
  - `beam` is a steerable beam you can aim, if something solid touches the beam trail, beam explodes. If another beam collides against other beam, creates a beam struggle.
  - `sbeam` is another steerable beam you can aim, but only when you hold the attack key. It doesn't work for beam struggles.
  - `hitscan` is used for instant shots and short range attacks, like shockwave, lightning and rail trail attacks.
  - `forcefield` creates a magnetic field or an explosion at the center of the attacker.

### weaponTime

- ```weaponTime [int]```<br/>
Attack timer, sets milliseconds for the attack, it fires after ending these milliseconds.

### randomWeaponTime

- ```randomWeaponTime [int]```<br/>
Sets and adds milliseconds for the [weaponTime](#weaponTime), randomizes attack timer calculation.

### kiCostAsPct

- ```kiCostAsPct [0/1]```<br/>
Enables ki cost percentage to calculate, see [kiPct](#kiPct).

### kiPct

- ```kiPct [0.0 - 1.0 (float)]```<br/>
Calculates maximum ki user multiplied by the ki percentage. Works if it's more than 0 and lesser than 1.

### kiCost

- ```kiCost [0-10000]```<br/>
Ki cost to reduce the user's ki. Works without ki percentage.

### chargeAttack

- ```chargeAttack [0/1]```<br/>
Enables the charge attack depending the ki charge points to release the attack above or equal the minimum ki charge points, see [minCharge](#minCharge).

### chargeAutoFire

- ```chargeAutoFire [0/1]```<br/>
Releases the attack with every charge points released after [weaponTime](#weaponTime) milliseconds.

### minCharge

- ```minCharge [0-6]```<br/>
Minimum ki charge points to release the attack with [chargeAttack](#chargeAttack).

### maxCharge

- ```maxCharge [0-6]```<br/>
Maximum ki charge points for the attack with [chargeAttack](#chargeAttack).

### damage

- ```damage [int]```<br/>
Attack damage.

### splashDamage

- ```splashDamage [int]```<br/>
Attack splash damage from the explosion.

### chargeDamageMult

- ```chargeDamageMult [int]```<br/>
Charge damage multiplier, calculates the damage multiplied per ki charge points of the attack.

### maxDamage

- ```maxDamage [int]```<br/>
Maximum damage deal for the attack.

### radius

- ```radius [int]```<br/>
Projectile radius.

### explosionRadius

- ```explosionRadius [int]```<br/>
Explosion radius.

### chargeRadiusMult

- ```chargeRadiusMult [int]```<br/>
Charge radius multiplier for the projectile, calculates the radius for the projectile multiplied per ki charge points of the attack.

### chargeExpRadiusMult

- ```chargeExpRadiusMult [int]```<br/>
Charge radius multiplier for the explosion, calculates the explosion radius multiplied per ki charge points of the attack.

### maxRadius

- ```maxRadius [int]```<br/>
Maximum radius for the projectile.

### maxExpRadius

- ```maxExpRadius [int]```<br/>
Maximum explosion radius.

### missileSpeed

- ```missileSpeed [int]```<br/>
Projectile speed.

### homing

- ```homing [0.0 - 1.0 (float)]```<br/>
Homing projectile trajectory. If lesser, rotates less. Works if it's more than 0 and lesser than 1.

### homingRange

- ```homingRange [float]```<br/>
Homing projectile range. The radius of the homing projectile to chase opponents.

### range

- ```range [float]```<br/>
Attack range. Applicable only to hitscan attack type.

### loopingAnim

- ```loopingAnim [0/1]```<br/>
Looping attack animation for the player model. If enabled, the player remains in the strike attack animation without returning to the prepare attack animation.

### noAttackAnim

- ```noAttackAnim [0/1]```<br/>
No attack animation for the player model. If enabled, the player remains in the strike attack animation skipping prepare attack animation even at the start.

### alternatingXOffset

- ```alternatingXOffset [0/1]```<br/>
Alternates projectile fire, switching from side to side: left to right, right to left and thus.

### randYOffset

- ```randYOffset [float]```<br/>
Randomizes the upward vector offset of projectile fire.

### randXOffset

- ```randXOffset [float]```<br/>
Randomizes the horizontal vector offset of projectile fire.

### coneOfFireX

- ```coneOfFireX [int]```<br/>
Sets the horizontal vector where the projectile is headed.

### coneOfFireY

- ```coneOfFireY [int]```<br/>
Sets the upward vector where the projectile is headed.

### piercing

- ```piercing [0/1]```<br/>
Enable piercing projectile. Applicable only to missile attack type, it won't work flawlessly for beam and sbeam attack types. Pierces the opponent by hitting 4 times if it stands in the same point. 

### reflective

- ```reflective [0/1]```<br/>
Enable reflective attack, reflects attacker' projectiles where the user aims. Applicable only to hitscan attack type with [range](#range).

### priority

- ```priority [int]```<br/>
Projectile priority. Not applicable to hitscan and forcefield attack types. Not applicable to two beam attack type collisions, these create a beam struggle. If more, breaks and explodes the opponent projectile. If both have the same priority, both break and explode.

### blinding

- ```blinding [0/1]```<br/>
The attack blinds the opponent during 6 seconds.

### missileGravity

- ```missileGravity [int]```<br/>
The projectile moves down depending on how much gravity is going down. Not applicable to hitscan and forcefield attack types.

### missileAcceleration

- ```missileAcceleration [float]```<br/>
Adds projectile acceleration, speeds up per frame. Not aplicable to hitscan and forcefield attack types. If between 0.0 and 1.0, deceleration.

### missileDuration

- ```missileDuration [int]```<br/>
Projectile lifetime. Not applicable to hitscan and forcefield attack types.

### multishot

- ```multishot [int]```<br/>
Number of projectiles per shot. Applicable only to missile attack type.

### bounces

- ```bounces [0/1]```<br/>
Enable projectile bouncing when colliding something solid. Applicable only to missile attack type.

### bounceFriction

- ```bounceFriction [float]```<br/>
Adds bounce friction. Applicable only to missile attack type. If lesser, less bounce force. Applicable only to missile attack type.

### noZBounce

- ```noZBounce [0/1]```<br/>
Fix the projectile vertical component to a deterministic bounce height, ignores [missileGravity](#missileGravity) property. Applicable only to missile attack type.

### extraKnockback

- ```extraKnockback [int]```<br/>
Adds knockback while the opponent deals damage from the attack. Lesser than 0, less knockback. If -1000, no knockback.

### railTrail

- ```railTrail [0/1]```<br/>
Instant rail trail, turns into like Q3 Railgun weapon. Applicable only to hitscan attack type. 

### movementPenalty

- ```movementPenalty [int]```<br/>
Movement penalty in seconds after using the attack, the user can't move after those seconds have passed. Applicable only to forcefield attack type.

### explosionSpawn

- ```explosionSpawn [weaponNum]```<br/>
Sets the splitting projectile from rdmissile attack type.

### end

- ```end```<br/>
Should be the last word in the *bfp_weapon.cfg* file.

## ORIGINAL BFP NOTES
- [blinding](#blinding) is only applicable to forcefield attack type.
- beam attack type with [chargeAutoFire](#chargeAutoFire) detonates at the first moment and attacker deals damage.
- rdmissile attack type with [chargeAutoFire](#chargeAutoFire) splits the projectile at the first moment and attacker deals damage.
- There's a bug on the beam attack type with [piercing](#piercing), players are stuck in the charging/firing status and their beam/sbeam can't shot from their muzzle and are coming from zeroed origin.
- As for ```usesGravity [0/1]```, it's UNUSED. Theoretically, it was used during the development of the original mod and was never applied to the gameplay.
