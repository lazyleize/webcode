include ../../Makefile.conf

OBJS= CIdentComm.o CIdentPub.o RSA.o BigInteger.o tlvtemplet.o CSnmpClient.o
LIBS = libidentcomm.a
all: $(LIBS)


$(LIBS):  $(OBJS)
	$(AR) $(AROPT) $@ $^
	
clean:
	rm -rf $(OBJS)  $(LIBS) 
	
%.o: %.cpp
	$(CXX) $(COMOPT) $(BASICINC) $<

