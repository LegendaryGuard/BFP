# Attacksets Config File

The attackset config file (*bfp_attacksets.cfg*) is used to set attacks to every player model prefix. 
Reads *bfp_attacksets.cfg* once in the server at the start after reading *bfp_weapon.cfg*.

### attackset

- ```attackset [int]```<br/>
Attackset identifier for this group.

### attack

- ```attack [attack index] [weaponNum]```<br/>
Sets the attack in this index, weaponNum is the attack identifier where helds attack properties.

### modelPrefix

- ```modelPrefix [string]```<br/>
Sets prefix for this player model group which starts with this prefix.

### defaultModel

- ```defaultModel [string]```<br/>
Sets default player model if it loads by using only the [modelPrefix](#modelPrefix) string in console.

### end

- ```end```<br/>
Should be the last word in the *bfp_attacksets.cfg* file.