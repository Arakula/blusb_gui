CXX = $(shell wx-config --cxx)
 
PROGRAM = blusb_gui
 
OBJECTS := $(patsubst %.cpp,%.o,$(wildcard *.cpp))
 
# implementation
 
.SUFFIXES:      .o .cpp
 
.cpp.o :
	$(CXX) -c `wx-config --cxxflags` -fpermissive `pkg-config libusb-1.0 --cflags` -o $@ $<
 
all:    $(PROGRAM)
 
$(PROGRAM):     $(OBJECTS)
	$(CXX) -o $(PROGRAM) $(OBJECTS) `wx-config --libs` `pkg-config libusb-1.0 --libs`
 
clean:
	rm -f *.o $(PROGRAM)
