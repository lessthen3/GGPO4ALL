# GGPO4ALL
A cross platform implementation of GGPO primarily meant for use within game engines so that you can run the code written by Tony "pond3r" Cannon natively without a compatibility layer or CLI

### THIS REPO IS A WORK IN PROGRESS AND STILL REQUIRES EXTENSIVE TESTING

## What's Different

__Removal of callbacks__

GGPO4ALL functions different from the original GGPO whereas instead of setting and forgetting some obscure function pointers and just letting GGPO rip, I instead elected to just expose functionality for doing the networking directly with functions that allow end-users to implement any custom networking logic they desire.

I find this is more in the spirit of how a library should work. With GGPO's old design you set function callbacks and just pray to god they do what you expect them to do.

Having a blackbox design like this just obscures control flow and makes integrating the library into a project more difficult than need be and kinda defeats the purpose of including third party software.

The purpose of a library is to offer functionality to an end-user to empower them to accomplish whatever end goal the software they are building does.

With this new design I hope it is easier to integrate GGPO4ALL into whatever project you are working on, while also giving more freedom over how the control flow of your network code works without adding much tech debt.

__GGPO4ALL does not handle any compression on your behalf anymore, any compression that you may need has to be handled before sending and after receiving packets__

I chose to remove zlib as a dependency because I think that letting the end-user decide which compression solution best suits their specific use case is overall a better design.

__GGPO4ALL does not handle serialization of data anymore__

Instead GGPO4ALL expects all data to be fed in as a byte stream of serialized (and possibly compressed) data. All state information will be held in a FrameState struct. GGPO4ALL will use this raw byte data to track overall state and any sync issues that may occur during runtime.

__Sessions__

GGPO4ALL uses a class object called Sessions that hold and manage state information. Each Session can be called on by multiple threads at a time, and multiple Sessions can be hosted on any number of threads. Packets are sent and received inside threadsafe queues that utilize mutexes.

__Takeaway__

These changes in my opinion make GGPO4ALL cleaner, more efficient, and leaves GGPO4ALL without any external depencies besides the cpp STL and OS specific API's.

## Building GGPO4ALL From Source

If you want to build GGPO4ALL for yourself:

0. This project is built using __C++20__, and you will need __CMake 3.20+__ (scroll down to the resources section for links if you are unfamiliar)

1. Clone the repo

2. Run: __python init.py [--debug or --release or --both] -G [desired_generator]__ in your terminal and your done!

The build output will be generated in __/build/(win or linux or osx)/(Debug or Release)__ as a static lib

## Resources:

[Latest CMake Download](https://cmake.org/download/)