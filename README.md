# Puyo VS [![Build](https://github.com/puyonexus/puyovs/actions/workflows/compile.yml/badge.svg)](https://github.com/puyonexus/puyovs/actions/workflows/compile.yml) [![Chat](https://img.shields.io/discord/991434636657971323?label=Chat)](https://discord.gg/cnUzgbFEXX)

Welcome to the Puyo VS 2 source code.

Puyo VS 2 was originally written by [Hernan](https://twitter.com/HernanZh/) with some contributions from other developers.

We hope that the community can benefit from this release and build a robust future for Puyo VS.

## Trademark Notice
"Puyo Puyo" is a registered U.S. trademark of SEGA CORPORATION since 2017. Puyo VS is not endorsed or related to SEGA CORPORATION in any way.

## Development Setup

Puyo VS is a C++ application using the Qt framework, built using the CMake build system. Therefore, you need a C++ compiler, a copy of CMake, and an installation of the Qt framework SDK (version 5) to build it.

Instructions for how to get started are provided here. The TL;DR is: Set up Qt SDK, set up CMake, run the install target once so that assets are available, and the Puyo VS binary is what you want to run.

### Windows

#### Setup Visual Studio 2022

If you have Winget, you can run the following in Windows Terminal:

```
winget install Microsoft.VisualStudio.2022.Community --silent --override "--wait --quiet --add ProductLang En-us --add Microsoft.VisualStudio.Workload.NativeDesktop --includeRecommended"
```

You can [install winget here](https://aka.ms/getwinget) if you don't have it.

#### Setup Qt SDK

We don't recommend them, but the [Qt official download page](https://www.qt.io/download) offers official installers for Qt for various platforms. They are discouraged because they require you to sign in.

If you want to avoid this, you can use one of our minimal Qt SDK builds:

1. [Download](https://github.com/puyonexus/qt-sdk-builder/releases/latest) the latest build for your platform. For example, on 64-bit Windows, you should use `x64-msvc-static-release` build.

2. Extract the archive to `C:\Qt` using your archiver of choice. (If you can't open the 7-zip file, you can [install 7-zip](https://www.7-zip.org/index.html) and use it.)

3. Find the bin directory of the extracted folder, for example `C:\Qt\Qt-5.15.2\bin`. It should contain a file called `qmake.exe`.

4. [Add the bin directory to your `PATH` environment variable](https://www.computerhope.com/issues/ch000549.htm). You may need to log out and log back in for this to fully take effect. In recent Windows versions, it should be enough to close apps (e.g. Windows Terminal) and start them again to have them see the new `%PATH%`.

You should be done. You can test to make sure by running `qmake --version` - if it prints a QMake version, then you have a Qt SDK installation on your `%PATH%`.

#### Build

##### Using the Command Line

You can build using the command line. Please note that CMake must be installed; you can install CMake with winget or chocolatey, or you can just [install it manually](https://cmake.org/download/). Visual Studio also comes with CMake, but it is only used when using the IDE. (Skip to the section below if you would prefer to use the Visual Studio IDE.)

```pwsh
# Configure build.
cmake -B .build

# Start build.
cmake --build .build --parallel 8 --config RelWithDebInfo

# Install assets (You only need to do this once.)
cmake --install .build --config RelWithDebInfo

# Run build!
.\.build\Client\RelWithDebInfo\PuyoVS.exe
```

> **Note**: You can use configurations other than RelWithDebInfo, but if you are making a Debug build, make sure you use a build of the Qt SDK that contains debug builds. MSVC debug objects have different CRT linkage and cannot be mixed with MSVC release objects. Alternatively, you can just manually specify to use the Release CRT by adding `/MD` to the compiler flags in CMake. Generally, we recommend using `RelWithDebInfo` unless you have a good reason to do otherwise.

##### Using Visual Studio 2022

You can also use Visual Studio 2022 to build and develop Puyo VS. Please keep in mind that the `.build` folder cannot be shared between ordinary CMake and Visual Studio 2022, so delete your `.build` folder prior to switching between them.

1. Go to <u>F</u>ile &rarr; <u>O</u>pen &rarr; C<u>M</u>ake, and open the `puyovs` source folder.

2. Run the "Build All" command (<u>B</u>uild &rarr; Build All, or Ctrl+Shift+B)

3. Run the "Install puyovs" command (<u>B</u>uild &rarr; Install puyovs). You only need to do this once.

4. Set the Startup Item to "PuyoVS.exe (Client\PuyoVS.exe)" and run it!

> **Note**: You may need to re-run code analysis after the build, so that auto-generated MOC and UIC files are included in the database. Otherwise, IntelliSense will output a lot of errors.

##### Using text editors and other CMake IDEs

Other IDEs and text editors that support CMake should work just fine. For most text editors, you will want to generate a `compile_commands.json` file and use the clangd LSP for good code completion.

### Linux

#### Install Dependencies

You need to install GCC, CMake and GNU Make (or, if you prefer, Ninja.) Puyo VS also depends on `libpulse`, `libasound2`, `libx11`, and Qt 5, so you need to install the development packages for these. The packages vary per distro, but on Ubuntu you can install them with:

```sh
sudo apt update
sudo apt install gcc cmake make libpulse-dev libasound2-dev libx11-dev qt5-default
```

On Arch, they can be installed with:

```sh
# This will upgrade your system - check archlinux.org for breakage info first.
# This is done to avoid unsupported "partial upgrades" that can render your system unusable.
# https://wiki.archlinux.org/title/system_maintenance#Partial_upgrades_are_unsupported
pacman -Syu cmake gcc make libx11 alsa-lib libpulse qt5-base
```

#### Building

You can build using the command line:

```pwsh
# Configure build.
cmake -B .build

# Start build.
cmake --build .build --parallel 8

# Install assets (You only need to do this once.)
sudo cmake --install .build

# Run build!
./.build/Client/PuyoVS.exe
```

### macOS

#### Setup XCode

You can install XCode from the Mac App Store. Once installed, you need to run it so that it can install additional tools. You also may need to run `xcode-select` to setup the command line tools.

#### Setup Qt SDK

We don't recommend them, but the [Qt official download page](https://www.qt.io/download) offers official installers for Qt for various platforms. They are discouraged because they require you to sign in.

If you want to avoid this, you can use one of our minimal Qt SDK builds:

1. [Download](https://github.com/puyonexus/qt-sdk-builder/releases/latest) the latest build for your platform. For example, on Intel Silicon macOS, you should use `x86_64-static-release` build.

2. Extract the archive using Archive Utility. You can move the resulting folder wherever you want, for example, your home folder.

3. Find the bin directory of the extracted folder, for example `/Users/john/Qt-5.15.2/bin`. It should contain a file called `qmake`.

4. Add the bin directory to your `$PATH` environment variable. You could do this by adding it to your local profile, using a command like: `echo 'export PATH=/Users/john/Qt-5.15.2/bin:$PATH' >> ~/.zprofile`. Note that for this to take effect, you will either need to start a new shell, or source the `.zprofile` by calling `. ~/.zprofile`.

You should be done. You can test to make sure by running `qmake --version` - if it prints a QMake version, then you have a Qt SDK installation on your `$PATH`.

#### Building

You can build using the command line:

```sh
# Configure build.
cmake -B .build

# Start build.
cmake --build .build --parallel 8

# Install assets (You only need to do this once.)
sudo cmake --install .build

# Run build!
open ./.build/Client/PuyoVS.app
```

## Source Organization

- Audiolib: An audio file loading, audio output and mixing library. Written by John Chadwick, but contains some vendored libraries.
- Audiotest: A small test program for Audiolib that plays an audio stream. Written by John Chadwick.
- Build: Folders where final builds are outputted to.
- BuildScripts: Build scripts, currently just for macOS. Written by BLueBLue.
- Client: The Qt-based frontend. Written by John Chadwick.
- ENet: A vendored copy of the [ENet](http://enet.bespin.org/) networking library.
- Inputlib: An input library. Written by John Chadwick.
- Inputtest: A small test program for Inputlib that outputs input events to the terminal. Written by John Chadwick.
- JsonCpp: A vendored copy of [JsonCpp](https://github.com/open-source-parsers/jsoncpp).
- Puyolib: The core game logic of Puyo VS 2. Written by Hernan.
- PVS_Enet: Puyo VS-specific networking code, wrapping ENet. Written by Hernan.
- SDL: A vendored copy of [SDL](https://www.libsdl.org/).
- Test: The assets that are bundled with builds.
- VgmStream: A vendored copy of [VgmStream](https://vgmstream.org/).
- Zlib: A vendored copy of [Zlib](https://www.zlib.net/).
