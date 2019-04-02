CXX = $(shell wx-config --cxx)
 
PROGRAM = blusb_gui
 
OBJECTS := $(patsubst %.cpp,%.o,$(wildcard *.cpp))
 
# implementation
 
.SUFFIXES:      .o .cpp
 
.cpp.o :
	$(CXX) -c `wx-config --cxxflags` -fpermissive -I/usr/include/libusb-1.0/ -o $@ $<
 
all:    $(PROGRAM)
 
$(PROGRAM):     $(OBJECTS)
	$(CXX) -o $(PROGRAM) $(OBJECTS) `wx-config --libs` -lusb-1.0
 
clean:
	rm -f *.o $(PROGRAM)
