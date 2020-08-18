# blt-nng-demo

## Table of contents

* [Description](#description)
* [Tech stack](#tech-stack)
* [How to build](#how-to-build)
* [How to use](#how-to-use)

## Description

This project is an example of using [nng](https://github.com/nanomsg/nng) in the app based
on [BearLibTerminal](http://foo.wyrd.name/en:bearlibterminal).

In fact, this app is a console-like client-server chat.\
It makes heavy use of things like multithreading and synchronization.

Unfortunately, there're nng binaries only for 64-bit Linux at the moment.\
If you're going to use this demo on another OS, you should build them by yourself
using instructions from the official repository.\
However, `./CMake/Findnng.cmake` is almost ready to the moment of adding binaries
for following operating systems:
* 32-bit Linux
* 32-bit Windows
* 64-bit Windows
* OS X

Binaries should be added in the same way as BearLibTerminal binaries
(see `./CMake/FindBearLibTerminal.cmake` and `./lib/BearLibTerminal`).

## Tech stack

This project uses:
* C++20
* [nng](https://github.com/nanomsg/nng)
* [nngpp](https://github.com/cwzx/nngpp) (modern C++ wrapper for nng)
* [BearLibTerminal](http://foo.wyrd.name/en:bearlibterminal)
* [date](https://github.com/HowardHinnant/date) (for showing dates and times)
* [CMake](https://cmake.org/) (for building)
* [Make](https://www.gnu.org/software/make/) (for building, running, etc)

## How to build

### Linux

1. Install following packages via package manager:
    * Make
    * CMake
    * gcc
    * g++
2. Run `make clean reload build` in the project root directory

### Windows

1. Install [MSYS2](http://repo.msys2.org/distrib/msys2-x86_64-latest.exe)
2. Run `pacman -Syu` in MinGW console
3. Re-open MinGW console
4. Run following commands in MinGW console:
    ```
    pacman -Su
    pacman -S mingw-w64-x86_64-cmake mingw-w64-x86_64-gcc mingw-w64-x86_64-pkg-config mingw-w64-x86_64-SDL2 mingw-w64-x86_64-make mingw-w64-x86_64-gdb
    ```
5. Add paths to directories with tools installed in the previous step to your system's `PATH` environment variable
6. Run `mingw32-make.exe clean reload build` in the project root directory

## How to use

### Linux

Run the following command in the project root directory:
```
# server
make exec_server REQ_REP_ENDPOINT=tcp://127.0.0.1:5555 PUB_SUB_ENDPOINT=tcp://127.0.0.1:5556

# client
make exec_client REQ_REP_ENDPOINT=tcp://127.0.0.1:5555 PUB_SUB_ENDPOINT=tcp://127.0.0.1:5556
```

### Windows

Run the following command in the project root directory:
```
# server
mingw32-make.exe exec_server REQ_REP_ENDPOINT=tcp://127.0.0.1:5555 PUB_SUB_ENDPOINT=tcp://127.0.0.1:5556

# client
mingw32-make.exe exec_client REQ_REP_ENDPOINT=tcp://127.0.0.1:5555 PUB_SUB_ENDPOINT=tcp://127.0.0.1:5556
```
