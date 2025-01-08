# Versioning Scheme

**NOTE: As Cacao Engine is still in very early active development, it has not had any releases and is versioned as `INDEV`.**  

## Versioning Format
Cacao Engine uses Evolutionary Versioning (EvoVer). This follows the format of `[Generation].[Major].[Minor].[Mini](-[Stage][Build])`. The suffix is in parentheses because it only appears for prereleases of the engine. Each release also has an associated nickname, see below for its rules. 

## Stages
* `a` - Alpha/nightly releases. Likely very unstable.
* `b` - Beta releases. Still possibly unstable.
* `rc` - Release candidates. Should be almost entirely stable.
* `s` - Stable releases. Communicates minor bugfix releases when increased past `1` for the first release.

## Number Incrementing
* Generation - Remains 0 during active development. When the first full release and subsequent versions are released, this number becomes the year when work on the release began.
* Major - Incremented for large new feature additions or large behavioral changes. This does not reset when the Generation number changes, except for the first full release.
* Minor - Incremented for smaller breaking changes.
* Mini - Incremented for non-breaking small feature additions.
* Build - Takes different formats depending on the Stage:
	* Stages `a` and `b` - Monotonic increases with numbers shared so as to identify where betas fall in the development cycle (e.g. a1, a2, b3, a4, b5, etc.)
	* Stage `rc` - Uses format `[a].[b]`, where `a` is the number of the stable release for which this is a candidate and `b` is the monotonically-increasing release candidate number.
	* Stage `s` - Monotonically increases for each release.

## Nicknames
Each release of Cacao Engine (excluding patches) receives a new nickname. The first letter of the nicknames goes down the alphabet with each minor version increment (wrapping back to `a` after reaching `z`), and each mini versiion increment gets a new nickname with the same first letter as the minor.

## Stability
All prerelease versions of Cacao Engine are **not** guaranteed to be stable and may not function correctly or at all on all devices/platforms. Once a prerelease has been tested on all platforms and functions correctly, that version will be promoted to the full release.  
All full release versions are stable and verified to work correctly on all platforms (though per-device compatibility may vary).