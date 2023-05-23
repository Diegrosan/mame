[![pipeline status](https://git.libretro.com/libretro/mame/badges/master/pipeline.svg)](https://git.libretro.com/libretro/mame/-/commits/master)
# **Libretro notice** #

Before sending bug reports to the upstream bug tracker, make sure the bugs are reproducible in the latest standalone release.

To build libretro MAME core from source you need to use `Makefile.libretro` make file:

```
make -f Makefile.libretro
```


For faster building after the initial makefile creation:

```
make -f Makefile.libretro PREMAKE=0
```

For Windows install `lld` for much faster linking.

--------

# **MAME** #

[![Join the chat at https://gitter.im/mamedev/mame](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/mamedev/mame?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

<<<<<<< HEAD
=======
Continuous integration build status:

| OS/Compiler                 | Status        |
| --------------------------- |:-------------:|
| Linux/clang and GCC         | ![CI (Linux)](https://github.com/mamedev/mame/workflows/CI%20(Linux)/badge.svg) |
| Windows/MinGW GCC and clang | ![CI (Windows)](https://github.com/mamedev/mame/workflows/CI%20(Windows)/badge.svg) |
| macOS/clang                 | ![CI (macOS)](https://github.com/mamedev/mame/workflows/CI%20(macOS)/badge.svg) |
| UI Translations             | ![Compile UI translations](https://github.com/mamedev/mame/workflows/Compile%20UI%20translations/badge.svg) |
| Documentation               | ![Build documentation](https://github.com/mamedev/mame/workflows/Build%20documentation/badge.svg) |
| BGFX Shaders                | ![Rebuild BGFX shaders](https://github.com/mamedev/mame/workflows/Rebuild%20BGFX%20shaders/badge.svg) |

Static analysis status for entire build (except for third-party parts of project):

[![Coverity Scan Status](https://scan.coverity.com/projects/5727/badge.svg?flat=1)](https://scan.coverity.com/projects/mame-emulator)

>>>>>>> mamedev/master
What is MAME?
=============

MAME is a multi-purpose emulation framework.

MAME's purpose is to preserve decades of software history. As electronic technology continues to rush forward, MAME prevents this important "vintage" software from being lost and forgotten. This is achieved by documenting the hardware and how it functions. The source code to MAME serves as this documentation. The fact that the software is usable serves primarily to validate the accuracy of the documentation (how else can you prove that you have recreated the hardware faithfully?). Over time, MAME (originally stood for Multiple Arcade Machine Emulator) absorbed the sister-project MESS (Multi Emulator Super System), so MAME now documents a wide variety of (mostly vintage) computers, video game consoles and calculators, in addition to the arcade video games that were its initial focus.

How to compile?
===============

If you're on a UNIX-like system (including Linux and macOS), it could be as easy as typing

```
make
```

for a full build,

```
make SUBTARGET=tiny
```

for a build including a small subset of supported systems.

See the [Compiling MAME](http://docs.mamedev.org/initialsetup/compilingmame.html) page on our documentation site for more information, including prerequisites for macOS and popular Linux distributions.

For recent versions of macOS you need to install [Xcode](https://developer.apple.com/xcode/) including command-line tools and [SDL 2.0](https://github.com/libsdl-org/SDL/releases/latest).

For Windows users, we provide a ready-made [build environment](http://www.mamedev.org/tools/) based on MinGW-w64.

Visual Studio builds are also possible, but you still need [build environment](http://www.mamedev.org/tools/) based on MinGW-w64.
In order to generate solution and project files just run:

```
make vs2019
```
or use this command to build it directly using msbuild

```
make vs2019 MSBUILD=1
```


Where can I find out more?
=============

* [Official MAME Development Team Site](https://www.mamedev.org/) (includes binary downloads, wiki, forums, and more)
* [Official MESS Wiki](http://mess.redump.net/)
* [MAME Testers](https://mametesters.org/) (official bug tracker for MAME and MESS)


Contributing
=============

## Coding standard

MAME source code should be viewed and edited with your editor set to use four spaces per tab. Tabs are used for initial indentation of lines, with one tab used per indentation level. Spaces are used for other alignment within a line.

Some parts of the code follow [Allman style](https://en.wikipedia.org/wiki/Indent_style#Allman_style); some parts of the code follow [K&R style](https://en.wikipedia.org/wiki/Indent_style#K.26R_style) -- mostly depending on who wrote the original version. **Above all else, be consistent with what you modify, and keep whitespace changes to a minimum when modifying existing source.** For new code, the majority tends to prefer Allman style, so if you don't care much, use that.

All contributors need to either add a standard header for license info (on new files) or inform us of their wishes regarding which of the following licenses they would like their code to be made available under: the [BSD-3-Clause](http://opensource.org/licenses/BSD-3-Clause) license, the [LGPL-2.1](http://opensource.org/licenses/LGPL-2.1), or the [GPL-2.0](http://opensource.org/licenses/GPL-2.0).

See more specific [C++ Coding Guidelines](https://docs.mamedev.org/contributing/cxx.html) on our documentation web site.


License
=======

The MAME project as a whole is made available under the terms of the
[GNU General Public License, version 2](http://opensource.org/licenses/GPL-2.0)
or later (GPL-2.0+), since it contains code made available under multiple
GPL-compatible licenses.  A great majority of the source files (over 90%
including core files) are made available under the terms of the
[3-clause BSD License](http://opensource.org/licenses/BSD-3-Clause), and we
would encourage new contributors to make their contributions available under the
terms of this license.

Please note that MAME is a registered trademark of Gregory Ember, and permission
is required to use the "MAME" name, logo, or wordmark.

<a href="http://opensource.org/licenses/GPL-2.0" target="_blank">
<img align="right" src="http://opensource.org/trademarks/opensource/OSI-Approved-License-100x137.png">
</a>

    Copyright (C) 1997-2021  MAMEDev and contributors

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License version 2, as provided in
    docs/legal/GPL-2.0.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
    more details.

Please see COPYING for more details.
