# Versioning Scheme

## Versioning Format
Cacao Engine uses a custom versioning system named Evolutionary Versioning (EvoVer). This follows the format of `[Generation].[Major].[Minor][Patch](-[Stage][Build])`. The suffix is in parentheses because it only appears for prereleases of the engine. Each release also has an associated nickname, see below for its rules. 

## Stages
* `a` - Alpha/nightly releases. Likely very unstable.
* `b` - Beta releases. Still possibly unstable.
* `rc` - Release candidates. Should be almost entirely stable.

## Number Incrementing
* Generation - Year during which work on the release began (so if the year rolls over during development, nothing changes until the next release), should not be incremented until the next major release after rollover to avoid confusion
* Major - Incremented for large new feature additions or any breaking changes, API or behavioral.
* Minor - Incremented for non-breaking changes or when the Patch rolls over.
* Patch - A lowercase letter, incremented from A to Z (the thinking is that after 26 patches, there's probably been enough changed to warrant a new minor release).
* Build - Stage-dependent:
	* Stages `a` and `b` - Monotonic increases with numbers shared between the two stages so as to identify where betas fall in the development cycle (e.g. a1, a2, b3, a4, b5, etc.) These numbers do not change when the Patch changes.
	* Stage `rc` - Monotonic increases with numbers not shared between stages or patch versions.

## Nicknames
Each new minor release of Cacao Engine is given a new nickname. The first letter of a nickname starts as A and will change every major release, cycling through to Z and then back to A. All minor releases are given a new nickname with the same first letter. Nicknames may not repeat. There is no rule on what must constitute a nickname.

## Stability
All prerelease versions of Cacao Engine are **not** guaranteed to be stable and may not function correctly or at all on all devices/platforms. Once a prerelease has been tested on all platforms and functions correctly, that version will be promoted to the full release.  
All full release versions are stable and verified to work correctly on all platforms (though per-device compatibility may vary).
