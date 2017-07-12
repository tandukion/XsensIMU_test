TARGET:= example
OBJLIBS	= libxcommunication libxstypes libxdevice
# CPP_FILES := $(wildcard *.cpp)
CPP_FILES := main.cpp # deviceclass.cpp
OBJECTS := $(CPP_FILES:.cpp=.o)
# OBJECTS+=conio.o
HEADERS = $(wildcard *.h)
INCLUDE=-I. -Iinclude -Isrc
CFLAGS=-g $(INCLUDE) -include config.h
CXXFLAGS=-std=c++11 $(CFLAGS)
# LFLAGS=-Lxcommunication -Lxstypes -lxcommunication -lxstypes -lpthread -lrt -ldl
LFLAGS=-Llib -lxdevice -lxcommunication -lxstypes -lpthread -lrt -ldl 

all : $(OBJLIBS) $(TARGET)

libxcommunication : libxstypes
	-$(MAKE) -C xcommunication $(MFLAGS)
	-cp xcommunication/libxcommunication.a lib/

libxstypes :
	-$(MAKE) -C xstypes $(MFLAGS) libxstypes.a
	-cp xstypes/libxstypes.a lib/

libxdevice :
	$(MAKE) -C src $(MFLAGS)	

$(TARGET): $(OBJECTS)
	$(CXX) $(CFLAGS) $(INCLUDE) $^  -o $@ $(LFLAGS) 

clean :
	$(RM) $(OBJECTS) $(TARGET)

clean_all :
	-$(RM) $(OBJECTS) $(TARGET)
	-$(MAKE) -C xcommunication $(MFLAGS) clean
	-$(MAKE) -C xstypes $(MFLAGS) clean
	- rm -rf lib/libxstypes.a lib/libxcommunication.a
	-$(MAKE) -C src $(MFLAGS) clean
