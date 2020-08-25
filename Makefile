#
# pgtc - Pete's Gnome Time Card is a gnome panel applet for 
# keeping track of hours
#
# Copyright (C) 2000 Pete Rijks
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2, or (at your option)
#  any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software Foundation,
#  Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  
#

CC = gcc
SRCS = pgtc.c about.c timing.c session.c ui.c
OBJS = $(SRCS:.c=.o)
TARGET = pgtc
CFLAGS = `gnome-config --cflags applets` -Wall -O2
LDFLAGS = `gnome-config --libs applets`

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS)

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f *~ *.o

install:
	cp $(TARGET) /usr/local/bin/
	cp $(TARGET).desktop /usr/share/gnome/apps/Utilities/
	cp $(TARGET).gnorba /etc/CORBA/servers/
