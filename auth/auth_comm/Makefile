include ../../Makefile.conf

OBJS= CAuthComm.o CAuthPub.o
LIBS = libauthcomm.a
all: $(LIBS)


$(LIBS):  $(OBJS)
	$(AR) $(AROPT) $@ $^
	
clean:
	rm -rf $(OBJS)  $(LIBS) 
	
%.o: %.cpp
	$(CXX) $(COMOPT) $(BASICINC) $<

