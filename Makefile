CC=gcc
CXX=g++
CFLAGS=-std=c99 -Wall -c -Wc++-compat -Ilib -I. -g
CXXFLAGS=-Wall -c -Ilib -I. -g
OBJDIR=build

SHARED := \
	$(OBJDIR)/src/loadProgram.o \
	$(OBJDIR)/src/loadBuffer.o \
	$(OBJDIR)/src/loadTexture.o \
	$(OBJDIR)/lib/png/lodepng.o \
	$(OBJDIR)/lib/pez/bstrlib.o \
	$(OBJDIR)/lib/pez/pez.o

UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
	SHARED := $(SHARED) $(OBJDIR)/lib/pez/pez.linux.o
	CXXFLAGS := $(CXXFLAGS)
	LIBS = -pthread -lX11 -lGL
endif

ifeq ($(UNAME), Darwin)
	CXXFLAGS := $(CXXFLAGS)
	CC = clang
	CXX = clang++
	SHARED := $(SHARED) $(OBJDIR)/lib/pez/pez.cocoa.o
	LIBS = -framework OpenGL -framework AppKit 
endif

all: $(OBJDIR) quadmesh

run: all
	./quadmesh

quadmesh:  $(OBJDIR)/src/main.o $(SHARED)
	$(CXX) $< $(SHARED) -o quadmesh $(LIBS)

$(OBJDIR): 
	@mkdir -p $@
	@mkdir -p $@/lib/pez
	@mkdir -p $@/lib/png
	@mkdir -p $@/src

$(OBJDIR)/%.o: %.cpp
	@echo $<
	$(CXX) $(CXXFLAGS) $< -o $@

$(OBJDIR)/%.o: %.cc
	@echo $<
	$(CXX) $(CXXFLAGS) $< -o $@

$(OBJDIR)/%.o: %.cxx
	@echo $<
	$(CXX) $(CXXFLAGS) $< -o $@

$(OBJDIR)/%.o: %.c
	@echo $<
	$(CC) $(CFLAGS) $< -o $@

$(OBJDIR)/%.o: %.m
	@echo $<
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f quadmesh 
	rm -rf $(OBJDIR)
