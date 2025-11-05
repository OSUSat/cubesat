# eps-firmware

This directory contains the firmware for the EPS board.

## Compilation Dependencies

- arm-none-eabi toolchain

## Submodule Initialization

```
git submodule update --init --recursive
```

## Building

```
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../arm-none-eabi-toolchain.cmake ..
make
```
```
