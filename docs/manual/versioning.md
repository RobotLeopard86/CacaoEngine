# Versioning Scheme

## Versioning Format
Cacao Engine uses a custom versioning system named Evolutionary Versioning (EvoVer). This follows the format of `[Year].[Major].[Minor][Patch](-[Series][Build])`. The suffix is in parentheses because it only appears for prereleases of the engine. Each release also has an associated nickname, see below for its rules. 

## Series
* `a` - Alpha releases, likely very unstable.
* `b` - Beta releases, still somewhat unstable.
* `rc` - Release candidates, should be fully stable.

## Number Incrementing
* Year - Year during which release preparation began, not incremented until the next major release after rollover to avoid confusion.
	* If rollover occurs before the first alpha release, then the year will be incremented.
	* 
* Major - Incremented for feature additions or any breaking changes, API or behavioral, starting from 1.
* Minor - Incremented for non-breaking changes or when the Patch rolls over, starting from 0.
* Patch - A lowercase letter, incremented from A to Z (the thinking is that after 26 patches, there's probably been enough changed to warrant a new minor release).
* Build - Incremented for each release in a series, starting from 0 and resetting when the series progresses.
	* The series may not progress until all work for the previous series is done (i.e. no alpha releases may be made once the beta series starts).

## Nicknames
Each new minor release of Cacao Engine is given a new nickname. The first letter of a nickname starts as A and will change every major release, cycling through to Z and then back to A. All minor releases are given a new nickname with the same first letter. Nicknames may not repeat. There is no rule on what must constitute a nickname.

## Stability
All prerelease versions of Cacao Engine are **not** guaranteed to be stable and may not function correctly or at all on all devices/platforms. Once a prerelease has been tested on all platforms and functions correctly, that version will be promoted to the full release.  
All full release versions are stable and verified to work correctly on all platforms (though per-device compatibility may vary).
