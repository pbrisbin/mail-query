# version
VERSION = $(shell git describe)

# paths
PREFIX ?= /usr/local
MANPREFIX ?= ${PREFIX}/share/man

# compiler flags
CC = c99
CPPFLAGS =
CFLAGS += -g -pedantic -Wall -Wextra ${CPPFLAGS}
LDFLAGS =

