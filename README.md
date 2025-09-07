# GGPO4ALL
A cross platform header only implementation of GGPO primarily meant for use within game engines so that you can run the code written by Tony "pond3r" Cannon and myself natively without a compatibility layer or CLI

>[!WARNING]
>GGPO4ALL is still in early alpha and extensive testing is still required. Work is being done to add features constantly, and the API is subject to breaking changes at any moment while work is being done to get GGPO4ALL to a complete 1.0 release.

## What's Different

#### Addition of TCP

GGPO originally only supported UDP but now GGPO4ALL supports TCP and UDP!

#### Removal of callbacks

GGPO4ALL functions different from the original GGPO whereas instead of setting and forgetting some obscure function pointers and just letting GGPO rip, I instead elected to just expose functionality for doing the networking directly with functions that allow end-users to implement any custom networking logic they desire.

I find this is more in the spirit of how a library should work. With GGPO's old design you set function callbacks and just pray to god they do what you expect them to do.

#### GGPO4ALL does not handle any compression on your behalf, any compression that you may need has to be handled before sending and after receiving packets

I chose to remove zlib as a dependency because I think that letting the end-user decide which compression solution best suits their specific use case is overall a better design.

#### GGPO4ALL does not handle serialization of data

Instead GGPO4ALL expects all data to be fed in as a byte stream of serialized (and possibly compressed) data. GGPO4ALL will use this raw byte data to track overall state and any sync issues that may occur during runtime.

#### Sessions

GGPO4ALL uses a class object called Sessions that hold and manage state information. Each Session can be called on by multiple threads at a time, and multiple Sessions can be hosted on any number of threads. Packets are sent and received inside threadsafe queues that utilize mutexes.

#### Takeaway 

These changes in my opinion make GGPO4ALL cleaner, more efficient, and leaves GGPO4ALL without any external depencies besides the cpp STL and OS specific API's.

## Design Philosophy

Having a blackbox design like the original GGPO just obscures control flow and makes integrating the library into a project more difficult than need be and kinda defeats the purpose of including third party software.

The purpose of a library is to offer functionality to an end-user to empower them to accomplish whatever end goal the software they are building has.

With this new design I hope it is easier to integrate GGPO4ALL into whatever project you are working on, while also giving more freedom over how the control flow of your network code works without adding much tech debt.

## Resources:

[Documentation](https://www.youtube.com/watch?v=dQw4w9WgXcQ)