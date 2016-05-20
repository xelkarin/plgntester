CXXFLAGS = -march=i686 -fomit-frame-pointer -ftracer -Os -Wall
LDFLAGS  = -s
HOST     = i686-w64-mingw32
CCPATH   = /usr/bin

ifeq ($(DEBUG),true)
CXXFLAGS = -march=i686 -g3 -Og -Wall
LDFLAGS  =
endif

CC  = $(CCPATH)/$(HOST)-gcc
CXX = $(CCPATH)/$(HOST)-g++
RC = $(CCPATH)/$(HOST)-windres

CXXFLAGS += -DUNICODE -D_UNICODE
LDFLAGS += -municode -mwindows -static-libgcc

TARGET = plgntester.exe
OBJS = plgntester.o
LDLIBS = -lcomctl32 -lole32

RES_SRC  = plgntester.rc
RES_OBJ = plgntester-rc.o

all: $(TARGET)

$(TARGET): $(OBJS) $(RES_OBJ)
	$(CC) $^ $(LDFLAGS) $(LDLIBS) -o $@

$(RES_OBJ): $(RES_SRC)
	$(RC) $(CPPFLAGS) -o $@ $<

clean:
	-rm $(TARGET) *.o

.PHONY: all clean
