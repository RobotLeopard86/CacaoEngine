# Platform Support

## Officially Supported
* Windows
* macOS
* GNU/Linux (most distros)

More operating systems/platforms may be supported in the future, though this is not guaranteed.

## Support Policy
* Windows platforms will have the current OS release supported for the latest two versions (e.g. Windows 11 23H2 and 24H2)
* macOS will be supported through the last two major OS releases for all patch versions (e.g. Sonoma and Sequoia)
    * macOS is supported for more releases than Windows because macOS receives yearly releases compared to Windows, which sticks on one "release" (e.g. Windows 11) with multiple yearly updates over its lifespan
* GNU/Linux distros will generally be supported for their last two releases in addition to the latest LTS version that is not in the previous category (including minor releases for distros that do that)
    * Example: Ubuntu 22.04, 24.04, and 24.10
    * Rolling release distros will be supported generally, but support may not be offered if the system is too out-of-date
    * For distros that do not release major versions often (e.g. Debian), support will only be offered for the latest version
    * For now, X11 and Wayland are both officially supported, but X11 support may be dropped in the future

## Currently Supported GNU/Linux Distros
Not every GNU/LINUX distro is supported, but most popular ones are:
* Debian and derivatives (e.g. Ubuntu, Linux Mint, Pop!_OS)
* Fedora and derivatives (e.g. Nobara)
    * As RHEL is based on comparatively old versions of Fedora, it is NOT supported
* Arch and derivatives (e.g. EndeavourOS, Garuda, Manjaro)  

As of now, SteamOS is in a bit of limbo with support, as the only way to run it off of the Steam Deck is with unofficial ISOs. It will generally be supported like most Arch derivatives, but SteamOS quirks may be difficult to resolve. If SteamOS is released onto desktop, it will be fully supported

## Currently Supported OS Versions
* Windows 11 23/24H2
* macOS 14.x (Sonoma) and 15.x (Sequoia)
* Ubuntu 22.04 (Jammy), 24.04 (Noble), and 24.10 (Oracular) and derivatives (look online for how to check what Ubuntu release your distro is based on)
* Fedora 40 and 41
* Arch and derivatives
* Debian 12 (Bookworm)