cppforo
=======

Cpp Foro: Forum software powered by C++

The project files and the complete build instructions will go up sooner or later, just waiting on some things.

It shouldn't take more than afew months or so.


Requirements
=====
Python 3.4

Windows (Linux is coming very soon!)


Building from source
=====
Building from source additionally requires the MySQL Connector/C++ driver and the Boost Library to be installed on the build machine.

Windows
====
Building the Boost Library
===
1) Download the latest version of the Boost Library from http://www.boost.org/

2) Open a command prompt (cmd.exe)

3) Navigate to the directory where you extracted the Boost Library with the cd command.

4) Run bootstrap.bat

5) Run b2 --toolset=msvc-10.0 architecture=x86 address-model=64 stage

Note: Change the toolset above to whatever compiler that you have on Windows and change the address model to whether you want a 32-bit build or a 64-bit build.
