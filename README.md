# blt-nng-demo

## Description

This project is an example of using [nng](https://github.com/nanomsg/nng) in the app based
on [BearLibTerminal](http://foo.wyrd.name/en:bearlibterminal).

In fact, this app is a console-like client-server chat.\
It makes heavy use of things like multithreading and synchronization.

Unfortunately, there're nng binaries only for 64-bit Linux at the moment.\
If you're going to use this demo on another OS, you should build them by yourself
using instructions from the official repository.\
Besides, `./CMake/Findnng.cmake` is almost ready to the moment of adding binaries
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

1. Install:
    * Make
    * CMake
    * gcc
    * g++
2. Run `make clean reload build` in the project root directory

### Windows

1. Install [MSYS2](http://repo.msys2.org/distrib/msys2-x86_64-latest.exe)
2. Install via MinGW console:
    * Make
    * CMake
    * gcc
    * g++
3. Add paths to directories with installed binaries to your system's `PATH` environment variable
4. Run `mingw32-make.exe clean reload build` in the project root directory

## How to use

### Linux

```
$ make exec_server REQ_REP_ENDPOINT=tcp://127.0.0.1:5555 PUB_SUB_ENDPOINT=tcp://127.0.0.1:5556
$ make exec_client REQ_REP_ENDPOINT=tcp://127.0.0.1:5555 PUB_SUB_ENDPOINT=tcp://127.0.0.1:5556
```

### Windows

```
$ mingw32-make.exe exec_server REQ_REP_ENDPOINT=tcp://127.0.0.1:5555 PUB_SUB_ENDPOINT=tcp://127.0.0.1:5556
$ mingw32-make.exe exec_client REQ_REP_ENDPOINT=tcp://127.0.0.1:5555 PUB_SUB_ENDPOINT=tcp://127.0.0.1:5556
```
