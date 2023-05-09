# Copyright (C) Alin Ichim 2023
CC = g++
LDFLAGS =
CFLAGS = -Wall -Wextra -g
SENAME = server
SUNAME = subscriber
SRCDIR = source
BUILDDIR = build
INCDIR = include
SRC = $(wildcard $(SRCDIR)/*.cpp)
OBJ = $(patsubst $(SRCDIR)/%.cpp, $(BUILDDIR)/%.o, $(SRC))

SE_SRC = $(SRCDIR)/logger.cpp $(SRCDIR)/conn_funcs.cpp
SU_SRC = $(SRCDIR)/logger.cpp $(SRCDIR)/conn_funcs.cpp 

SE_OBJ = $(patsubst $(SRCDIR)/%.cpp, $(BUILDDIR)/%.o, $(SE_SRC))
SU_OBJ = $(patsubst $(SRCDIR)/%.cpp, $(BUILDDIR)/%.o, $(SU_SRC))

.PHONY: clean $(SENAME) $(SUNAME)

all: $(SENAME) $(SUNAME)

$(shell mkdir -p $(BUILDDIR))

$(SENAME): $(BUILDDIR)/$(SENAME).o $(SE_OBJ)
	$(CC) -o $@ $(LDFLAGS) $^

$(SUNAME): $(BUILDDIR)/$(SUNAME).o $(SU_OBJ)
	$(CC) -o $@ $(LDFLAGS) $^

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) -c -I./$(INCDIR) -o $@ $<

clean:
	rm -rf $(SENAME) $(SUNAME) $(OBJ)