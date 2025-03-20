# GGPO4ALL
A cross platform implementation of GGPO primarily meant for use within game engines so that you can run the code written by Tony "pond3r" Cannon natively without a compatibility layer or CLI

### THIS REPO IS A WORK IN PROGRESS AND STILL REQUIRES EXTENSIVE TESTING

## What's Different

__Removal of callbacks__

GGPO4ALL functions different from the original GGPO whereas instead of setting and forgetting some obscure function pointers and just letting GGPO rip, I instead elected to just expose functionality for doing the networking directly with functions that allow end-users to implement any custom networking logic they desire.

I find this is more in the spirit of how a library should work. With GGPO's old design I find that you're supposed to set these function callbacks and just pray to god they do what you expect them to do.

Having a blackbox design like this just obscures control flow and makes integrating the library into a project more difficult than need be and kinda defeats the purpose of including third party software.

The purpose of a library is to offer functionality to an end-user to empower them to accomplish whatever end goal the software they are building does.

With this new design I hope it is easier to integrate GGPO4ALL into whatever project you are working on, while also giving more freedom over how the control flow of your network code works without adding any tech debt.

__GGPO4ALL does not handle any compression on your behalf anymore, any compression that you may need has to be handled before and after sending packets__

I chose to remove zlib as a dependency because I think that letting the end-user decide which compression solution best suits their specific use case is overall a better design.

This in my opinion is cleaner, more efficient, and leaves GGPO4ALL without any external depencies besides the cpp STL.

__IMPORTANT:__ GGP4OALL is only meant to be called on by a single thread. This is subject to change, but for now GGPO4ALL is meant to be used on a network thread or on the main thread of a game engine.

## Building GGPO4ALL From Source

If you want to build GGPO4ALL for yourself:

0. This project is built using __C++20__, and you will need __CMake 3.20+__ (scroll down to the resources section for links if you are unfamiliar)

1. Clone the repo

2. Run: __python init.py [--debug or --release or --both] -G [desired_generator]__ in your terminal and your done!

The build output will be generated in __/build/(win or linux or osx)/(Debug or Release)__ as a static lib

## Resources:

[Latest CMake Download](https://cmake.org/download/)