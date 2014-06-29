## Right now this Makefile is really brittle, if it fails during a build then you need to make clean; and restart 

SHELL=/bin/sh
.SUFFIXES:
.SUFFIXES: .cc .cxx .o
VPATH=src:include:build:build/lib:../src:/include:../include:lib
CC = g++
LIBDIR=/usr/local/lib
ROOTLIB=$(shell root-config --libdir)
ROOTCFLAGS=$(shell root-config --cflags)
ROOTLFLAGS=$(shell root-config --ldflags)
ROOTLIBS=$(shell root-config --libs) -lSpectrum -lGui
#DEBUG=-ggdb
#PROFILE=-pg
CXXFLAGS=-Wall -fPIC -O2 $(PROFILE) -rdynamic -I$(CURDIR)/include
SHAREDCFLAGS = -shared $(ROOTCFLAGS) $(CXXFLAGS)
NONSHARED = -c -pipe -Wshadow -W -Woverloaded-virtual $(ROOTCFLAGS) $(CXXFLAGS) -DR__HAVE_CONFIG
BUILDDIR = $(CURDIR)/build
LIB = $(BUILDDIR)/lib

all: dirs pademw

dirs:
	test -d $(LIB) || mkdir -p $(LIB)

$(eval WI = $(realpath $(LIB)/controller.so))
pademw: pademw.cpp controller.so 
ifeq ($(WI),)
	$(CC) $(CXXFLAGS) $(ROOTCFLAGS) $(ROOTLIBS) $(filter-out %.so, $^) $(abspath $(patsubst %.so, $(LIB)/%.so,$(filter %.so, $(notdir $^))))  -o $@ -levent  #-lprofiler
else
	$(CC) $(CXXFLAGS) $(ROOTCFLAGS) $(ROOTLIBS) $(filter-out %.so, $^) $(abspath $(patsubst %.so, $(LIB)/%.so,$(filter %.so, $(notdir $^))))  -o $@ -levent  #-lprofiler
endif
	mv $@ build/


controller.so: controller.cpp
	$(CC) $(SHAREDCFLAGS) $(ROOTCFLAGS) $(ROOTLIBS) $^ -o $@
	mv $@ build/lib

clean: 
	rm -rf build/*

