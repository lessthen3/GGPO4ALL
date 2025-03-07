# GGPO4ALL
A cross platform port of GGPO primarily meant for use within game engines so that you can run the code written by Tony Cannon (aka pond3r) natively without a compatibility layer or CLI

### THIS REPO IS A WORK IN PROGRESS AND STILL REQUIRES EXTENSIVE TESTING

## Building GGPO4ALL From Source

If you want to build Peach-E for yourself:

0. This project is built using __C++20__, and you will need __CMake 3.20+__ (scroll down to the resources section for links if you are unfamiliar)

1. Clone the repo

2. Run: __python init.py --debug or --release or --both -G [desired_generator]__ in your terminal and your done!

The build output will be generated in __/build/(win or linux or osx)/(Debug or Release)__ as a static and dynamic lib

## Resources:

[Latest CMake Download](https://cmake.org/download/)