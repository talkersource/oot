#
# Copyright 1996 by Dave Jarvis
#
# Makefile for OOT - The Object-Oriented Talker
#
CC = gcc
CCOPTS = -O2 -static
PROGRAM = oot

############## Different OS Library Files ##############
#
# Remove the # before the line which matches your operating system,
# and place a # before the line which doesn't match.
#
# Linux / SunOS v4.0
LIBS = -lg++ -lstdc++ -liostream -liberty
#
# BSDI 2.1
#LIBS = -lg++ -lstdc++
#
# FreeBSD 2.1.0
#LIBS = -lg++
#

OBJS = cmdparse.o colour.o command.o common.o \
	filename.o \
	handystr.o \
	listbase.o \
	main.o message.o \
	profile.o \
	room.o \
	say.o socket.o \
	talker.o telnet.o ttime.o \
	user.o

all: $(PROGRAM)

oot: $(OBJS)
	$(CC) $(CCOPTS) -o $(PROGRAM) $(OBJS) $(LIBS)
	strip $(PROGRAM)

message.o:
	$(CC) $(CCOPTS) -c message.C
cmdparse.o:
	$(CC) $(CCOPTS) -c cmdparse.C
colour.o:
	$(CC) $(CCOPTS) -c colour.C
command.o:
	$(CC) $(CCOPTS) -c command.C
filename.o:
	$(CC) $(CCOPTS) -c filename.C
handystr.o:
	$(CC) $(CCOPTS) -c handystr.C
listbase.o:
	$(CC) $(CCOPTS) -c listbase.C
main.o:
	$(CC) $(CCOPTS) -c main.C
room.o:
	$(CC) $(CCOPTS) -c room.C
say.o:
	$(CC) $(CCOPTS) -c say.C
socket.o:
	$(CC) $(CCOPTS) -c socket.C
talker.o:
	$(CC) $(CCOPTS) -c talker.C
ttime.o:
	$(CC) $(CCOPTS) -c ttime.C
user.o:
	$(CC) $(CCOPTS) -c user.C
common.o:
	$(CC) $(CCOPTS) -c common.C
profile.o:
	$(CC) $(CCOPTS) -c profile.C
telnet.o:
	$(CC) $(CCOPTS) -c telnet.C
