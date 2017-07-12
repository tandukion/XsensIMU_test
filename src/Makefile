TARGET := ../lib/libxdevice.a
CPP_FILES := deviceclass.cpp
C_FILES := conio.c
OBJECTS := $(CPP_FILES:.cpp=.cpp.o) $(C_FILES:.c=.c.o)
CFLAGS = -g -I. -I../include
CXXFLAGS+= -std=c++11 $(CFLAGS)

all : $(TARGET)


$(TARGET): $(OBJECTS)
	ar rcs $@ $^

%.cpp.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<
	@$(CXX) -MM $(CXXFLAGS) $< > $*.dpp
	@mv -f $*.dpp $*.dpp.tmp
	@sed -e 's|.*:|$*.cpp.o:|' < $*.dpp.tmp > $*.dpp
	@sed -e 's/.*://' -e 's/\\$$//' < $*.dpp.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.dpp
	@rm -f $*.dpp.tmp

%.c.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
	@$(CC) -MM $(CFLAGS) $< > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|$*.c.o:|' < $*.d.tmp > $*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

clean:
	-$(RM) $(TARGET)
	-$(RM) *.o *.d
