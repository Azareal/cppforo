Project CppForo
=======

Project CppForo: Forum software powered by C++. An experimental project to show the viability of C++ in the forum software market and to explore the potential of C++ on the forum market.

We currently only have build instructions for Ubuntu. The complete build instructions for Windows are coming soon, although the current focus is on Linux.


Requirements
=====
Python 3.4

Windows or Linux (tested on Ubuntu)


Building from source
=====
Building from source additionally requires the MySQL Connector/C++ driver, the Pion HTTP Library, the Crypto++ cryptography library for enhanced password security and the Boost Library to be installed on the build machine.

Ubuntu
=====
Run the following commands to install the dependencies that the software needs and their dependencies:

sudo apt-get install build-essential module-assistant dh-autoreconf

sudo apt-get install zlib1g-dev libssl-dev libxml2-dev libbz2-dev libpcap-dev python-lxml libpython3.4-dev libmysqlcppconn-dev

sudo apt-get install libboost1.54-all-dev

cd over to the pion base directory.

./autogen.sh

./configure

make all

sudo make install

If the following command doesn't work then, you may need to replace the 9 with whatever the latest version of Crypto++ is:

sudo apt-get install libcrypto++9 libcrypto++9-dbg libcrypto++-dev


Install MariaDB or MySQL.

MariaDB:

sudo apt-get install mariadb-server


Open up the Code::Blocks project and compile it. A method that doesn't require this IDE is coming soon..

Windows
====

Windows build instructions are coming in the future, however here's some information on building the Boost Library, as the official documentation lacks detail on some things:

Building the Boost Library
===
1. Download the latest version of the Boost Library from http://www.boost.org/
2. Open a command prompt (cmd.exe)
3. Navigate to the directory where you extracted the Boost Library with the cd command.
4. Run bootstrap.bat
5. Run b2 --toolset=msvc-10.0 architecture=x86 address-model=64 stage

Note: Change the toolset above to whatever compiler that you have on Windows and change the address model to whether you want a 32-bit build or a 64-bit build.
