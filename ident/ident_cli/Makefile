include ../../Makefile.conf

COMMINC= ../ident_comm
OBJS=$(patsubst %.cpp, %.o, $(wildcard *.cpp))

LIBS = libidentrelayapi.a

all: $(LIBS) 
	
$(LIBS):  $(OBJS)
	$(AR) $(AROPT) $@ $^

clean:
	rm -rf $(OBJS)  $(LIBS) 

%.o: %.cpp
	$(CXX) $(COMOPT) $(BASICINC)-I$(COMMINC) $<
