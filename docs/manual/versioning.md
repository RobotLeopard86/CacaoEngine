# Versioning Scheme

**NOTE: As Cacao Engine is still in very early active development, it has not had any releases and is versioned as `INDEV`.**  

## Stability
All prerelease versions of Cacao Engine are **not** guaranteed to be stable and may not function correctly or at all on all devices/platforms. Once a prerelease has been tested on all platforms and functions correctly, that version will be promoted to the full release.  
All full release versions are stable and verified to work correctly on all platforms (though per-device compatibility may vary).

## Format
Cacao Engine follows the format of `[Series].[Major].[Minor][Patch](-pre[Prerelease])`. The prerelease suffix is in parentheses because it only appears for prereleases of the engine. Each release also has an associated nickname, see below for its rules. The patch value is a letter, unlike the others, beginning at `a` and incrementing to `z`. The thinking is that if 26 patches have been made, it's probably time for a new minor release.

## Number Incrementing
* Series - Incremented for large leaps forward in engine capabilities.
* Major - Incremented for breaking API or behavioral changes.
* Minor - Incremented for features added that do not change the behavior of any existing functionality.
* Patch - Incremented for minor bugfix updates.
* Prerelease - Incremented for each new prerelease for a given version.

## Nicknames
Each release of Cacao Engine (excluding patches) receives a new nickname. The first letter of the nicknames goes down the alphabet with each major release (wrapping back to `a` after reaching `z`), and each minor release gets a new nickname with the same first letter as the major.