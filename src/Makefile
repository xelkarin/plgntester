CXXFLAGS = -march=i686 -g3 -Og -Wall
HOST     = i686-w64-mingw32

CCPATH   = /usr/bin
CC  = $(CCPATH)/$(HOST)-gcc
CXX = $(CCPATH)/$(HOST)-g++
RC = $(CCPATH)/$(HOST)-windres

CXXFLAGS += -DUNICODE -D_UNICODE
LDFLAGS = -municode -static-libgcc

TARGET = plgntester.exe
OBJS = plgntester.o

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
