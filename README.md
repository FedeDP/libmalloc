# Alloc

## What's this

Small library re-implementation of malloc family of functions.  
It makes use of mmap and mremap (thus, for mremap support, requires _GNU_SOURCE).  

Internally, It uses a linked list to store memory mappings; header data weights 24B for each memory chunk.

## Why?

Because I had free time obviously. And it is a short but nice journey into some linux memory management details.  

## How to build

It uses cmake, thus:

    $ mkdir build
    $ cd build
    $ cmake ../
    $ make

## CMake parameters

* -DBUILD_TESTS=true to build tests too (it uses cmocka + valgrind to test for memleaks)
* -DENABLE_PROT=bool to enable/disable mprotect calls on freed memory chunks (disabled by default for performance reasons)

**Please do not use this!**
