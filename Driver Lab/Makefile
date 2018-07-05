
# Hello Makefile
# --------------
# This is a minimal kernel makefile that creates 3 variants of the 'Hello Word'
# modules.  See http://www.tldp.org/LDP/lkmpg/2.6/html/index.html
# for the original versions of this (for linux 2.6!)

# declare the object files for the module
obj-m += adder.o

# build by calling make in the current linux version of the build directory
# Note: the `uname -r` command returns the current version of linux.
# This causes the current modules to be included in the build with this
# directory added to the set of module sources.

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
