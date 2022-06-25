# Puyo Puyo VS

Welcome to the Puyo Puyo VS 2 source code.

Puyo Puyo VS 2 was originally written by [Hernan](https://twitter.com/HernanZh/) with some contributions from other developers.

We hope that the community can benefit from this release and build a robust future for Puyo Puyo VS.

## Trademark Notice
"Puyo Puyo" is a registered U.S. trademark of SEGA CORPORATION since 2017. Puyo Puyo VS is not endorsed or related to SEGA CORPORATION in any way.

## Setup Qt

Puyo Puyo VS 2 is written in C++ using the Qt framework. You'll need some form of the Qt SDK to run it. Currently, we're using Qt 5 to compile Puyo Puyo VS 2, although Qt 4 will still work for now. (Qt 6 is not supported yet.) We're also using qmake rather than CMake for the time being.

For Linux users, it is generally advised to consult your package manager. For example, on Ubuntu, you can install `qt5-default` and `qtcreator` to get a basic Qt development environment. Exactly how to configure the Qt SDK for Linux is out of the scope of this README.

For macOS and Windows users, you have several options to get a copy of the Qt SDK. The [Qt official download page](https://www.qt.io/download) offers official installers for Qt for various platforms. Please note that these generally require you to sign up for a Qt account. We don't encourage this unless you already have a Qt account. We also have [minimal builds of the Qt SDK](https://github.com/puyonexus/qt-sdk-builder/) which are used for our automated builds; you can use these locally if you want, but they may not be compatible with Qt Creator.

(In the future, we may migrate off of Qt or use Qt via vcpkg instead.)

## Building

Once you have a Qt SDK setup, you can build a copy of Puyo VS by building the Client subproject.

If you prefer the terminal, you can setup a quick build like so:

```sh
# Set up build directory outside of the source code
mkdir ../build && cd ../build

# Run qmake to configure a build for this build dir. You can also specify debug.
qmake ../puyovs CONFIG+=release

# Run make. The value you specify for -j specifies maximum parallel processes to compile with.
make -j8

# On Windows, if you are using MSVC, you should use nmake or jom.
# You can install jom via chocolately. It is recommended.
# MinGW is not recommended, but if you use msys2 you should be able to run `make` normally.
# Make sure you are using a shell setup for MinGW!
# MSys has multiple shell options and some of them are NOT for use with MinGW!

# Assuming all is well, you can run your new shiny PuyoVS executable.
cd ../puyovs/Build/Release
./PuyoVS

# On macOS, you'll need to do something like
# $ open ./PuyoVS.app
# And on Windows, something like
# $ .\PuyoVS.exe
```

**Want code completion?** Use Qt Creator for easy code completion. If you want code completion in other code editors, like Vim or VSCode, it is recommended to use `clangd` with a program that can generate a compile commands metadata file, like [bear](https://github.com/rizsotto/Bear). In the future, CMake should make this easier. Some day! Maybe...

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
- Puyolib: The core game logic of Puyo Puyo VS 2. Written by Hernan.
- PVS_Enet: Puyo VS-specific networking code, wrapping ENet. Written by Hernan.
- SDL: A vendored copy of [SDL](https://www.libsdl.org/).
- Test: The assets that are bundled with builds.
- VgmStream: A vendored copy of [VgmStream](https://vgmstream.org/).
- Zlib: A vendored copy of [Zlib](https://www.zlib.net/).
