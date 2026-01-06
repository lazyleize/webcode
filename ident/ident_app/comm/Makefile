include ../../../Makefile.conf

GYPAYINC=../../ident_comm
AUTHREL=../../../auth/auth_cli
CREREL=../../ident_cli

AR = ar
AROPT=-scurv

OBJ = CIdentAppComm.o

.PHONY: all clean

all: libidentappcomm.a 


libidentappcomm.a :CIdentAppComm.o CIdentAppTools.o
	$(AR) $(AROPT) $@ $^

clean:
	rm -f *.o *.a
	
%.o: %.cpp
	$(CXX) $(COMOPT) $(BASICINC) -I$(AUTHREL) -I$(CREREL) -I$(GYPAYINC) $<
