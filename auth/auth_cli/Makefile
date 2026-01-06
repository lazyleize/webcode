include ../../Makefile.conf

OBJS=$(patsubst %.cpp, %.o, $(wildcard *.cpp))

LIBS = libauthrelayapi.a

all: $(LIBS) 
	
$(LIBS):  $(OBJS)
	$(AR) $(AROPT) $@ $^

clean:
	rm -rf $(OBJS)  $(LIBS) 

%.o: %.cpp
	$(CXX) $(COMOPT) $(BASICINC) $<
