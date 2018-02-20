#
# Created:  Fri 09 Feb 2018 06:31:34 PM PST
# Modified: Mon 19 Feb 2018 07:21:16 PM PST
#
# Copyright 2018 (C) Robert Gill
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

CFLAGS   = -march=i686 -g3 -Og -Wall
CXXFLAGS = -march=i686 -g3 -Og -Wall
HOST     = i686-w64-mingw32

CCPATH = /usr/bin
CC  = $(CCPATH)/$(HOST)-gcc
CXX = $(CCPATH)/$(HOST)-g++
RC = $(CCPATH)/$(HOST)-windres

UNICODE := true

ifeq ($(UNICODE),true)
UNICODE_CFLAGS  = -DUNICODE -D_UNICODE
UNICODE_LDFLAGS = -municode
endif

CFLAGS += $(UNICODE_CFLAGS)
CXXFLAGS += $(UNICODE_CFLAGS)
LDFLAGS = $(UNICODE_LDFLAGS) -static-libgcc

TARGET = plgntester.exe
OBJS = plgntester.o pluginapi.o

RES_SRC  = plgntester.rc
RES_OBJ = plgntester-rc.o

all: $(TARGET)

$(TARGET): $(OBJS) $(RES_OBJ)
	$(CXX) $^ $(LDFLAGS) -o $@

$(RES_OBJ): $(RES_SRC)
	$(RC) $(CPPFLAGS) -o $@ $<

clean:
	-rm $(TARGET) *.o

.PHONY: all clean
