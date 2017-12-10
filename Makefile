##
## Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
## Creation Date: Sat Nov 25 17:39:35 PST 2017
## Last Modified: Sat Nov 25 17:39:38 PST 2017
## Syntax:        GNU Makefile
## Filename:      pianoroll/Makefile
## vim:           ts=3
##
## Description: Makefile to run tasks for library.
##
## To compile:
##     make
##

# Set the environmental variable $MACOSX_DEPLOYMENT_TARGET to
# "10.9" in Apple OS X to compile for OS X 10.9 and later (for example,
# you can compile for OS X 10.9 computers even if you are using the 10.10
# version of the operating system).
ENV =

OS := $(shell uname -s)

ifeq ($(OS),Darwin)
	OS = OSX
	# Minimum OS X Version for C++11 is OS X 10.9:
   ENV = MACOSX_DEPLOYMENT_TARGET=10.9
   # use the following to compile for 32-bit architecture on 64-bit comps:
   #ARCH = -m32 -arch i386
else
   # use the following to compile for 32-bit architecture on 64-bit comps:
   # (you will need 32-bit libraries in order to do this)
   # ARCH = -m32
endif


###########################################################################
#                                                                         #
# Beginning of user-modifiable configuration variables                    #
#                                                                         #

BINDIR        = bin
OBJDIR        = obj
SRCDIR        = src
TOOLDIR       = tools
SRCDIR_MIN    = src
INCDIR        = include
INCDIR_MIN    = include
LIBDIR        = lib
LIBFILE       = libpianoroll.a
AR            = ar
RANLIB        = ranlib

PREFLAGS  = -c -g $(CFLAGS) $(DEFINES) -I$(INCDIR) 
PREFLAGS += -O3 -Wall

# using C++ 2011 standard in Humlib:
PREFLAGS += -std=c++11 -fext-numeric-literals

# Add -static flag to compile without dynamics libraries for better portability:
POSTFLAGS =
# POSTFLAGS += -static

COMPILER      = LANG=C $(ENV) g++ $(ARCH)
# Or use clang++ v3.3:
#COMPILER      = clang++
#PREFLAGS     += -stdlib=libc++

#                                                                         #
# End of user-modifiable variables.                                       #
#                                                                         #
###########################################################################


# setting up the directory paths to search for dependency files
vpath %.h   $(INCDIR):$(SRCDIR)
vpath %.cpp $(SRCDIR):$(INCDIR)
vpath %.o   $(OBJDIR)

# generating a list of the object files
OBJS  =
OBJS += $(notdir $(patsubst %.cpp,%.o,$(wildcard $(SRCDIR)/tool-*.cpp)))
OBJS += $(notdir $(patsubst %.cpp,%.o,$(wildcard $(SRCDIR)/[A-Z]*.cpp)))

# targets which don't actually refer to files
.PHONY: examples myprograms src include dynamic tools


###########################################################################


all: library tools

library: lib
lib: makedirs $(OBJS)
	@-rm -f $(LIBDIR)/$(LIBFILE)
	@$(AR) r $(LIBDIR)/$(LIBFILE) $(addprefix $(OBJDIR)/, $(OBJS))
	@$(RANLIB) $(LIBDIR)/$(LIBFILE)


tool: tools
tools:
	@$(MAKE) -f Makefile.programs


clean:
	@echo Erasing object files...
	@-rm -f $(OBJDIR)/*.o
	@echo Erasing obj directory...
	@-rmdir $(OBJDIR)


superclean: clean
	-rm -rf $(LIBDIR)
	-rm -f  $(BINDIR)/test*


makedirs:
	@-mkdir -p $(OBJDIR)
	@-mkdir -p $(LIBDIR)


%:
	@echo 'if [ "$<" == "" ]; then $(MAKE) -f Makefile.programs $@; fi' | bash -s


pugixml.o: pugixml.cpp
	@echo [CC] $@
	@$(COMPILER) $(PREFLAGS) -o $(OBJDIR)/$(notdir $@) $(POSTFLAGS) $<

###########################################################################
#                                                                         #
# defining an explicit rule for object file dependencies                  #
#                                                                         #

%.o : %.cpp 
	@echo [CC] $@
	@$(COMPILER) $(PREFLAGS) -o $(OBJDIR)/$(notdir $@) $(POSTFLAGS) $<

#                                                                         #
###########################################################################



###########################################################################
#                                                                         #
# Dependencies -- generated with the following command in                 #
#      the src directory (in bash shell):                                 #
#                                                                         #
# for i in src/*.cpp
# do
#    cc -std=c++11 -Iinclude -MM $i | sed 's/include\///g; s/src\///g'
#    echo ""
# done
#

