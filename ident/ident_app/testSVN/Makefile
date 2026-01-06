include ../../../Makefile.conf

LIB_TO_BUILD = new_app_GetSVN_Firmware

#target
#----------------------
TARGET = $(LIB_TO_BUILD)

AUTHREL= ../../../auth/auth_cli
THIS_COMPILE += -I$(AUTHREL)
THIS_LINK += -L$(AUTHREL)

COMMREL= ../../ident_cli
THIS_COMPILE += -I$(COMMREL)
THIS_LINK += -L$(COMMREL)

COMMPATH= ../../ident_comm
THIS_COMPILE += -I$(COMMPATH)
THIS_LINK += -L$(COMMPATH)

COMMAPPINC= ../comm
THIS_COMPILE += -I$(COMMAPPINC)

THIS_COMPILE += $(BASICINC)
THIS_COMPILE += $(COMOPT)

COMMAPPLIB= ../comm
THIS_LINK += -L$(COMMAPPLIB)

APPLINK= -lidentappcomm -lidentcomm -lidentrelayapi -liconv
THIS_LINK += -L$(APPLINK)

THIS_LINK += -L$(BASICLIB)
THIS_LINK += -L$(CGILIBLINK)
THIS_LINK += -L$(BASICLIBLINK)

#----------------------------
ALLSRC = $(wildcard *.cpp)
ALL_OBJ = $(patsubst %.cpp, $(BUILD_CACHE_DIR)/%.o, $(filter %.cpp, $(ALLSRC)))
#----------------------------
INSTALL_PATH = ../bin/

.PHONY:all
all: createDir $(TARGET)

createDir:
	@mkdir -p $(BUILD_CACHE_DIR)/ 2>/dev/null
	@mkdir -p $(INSTALL_PATH)/ 2>/dev/null 

new_app_GetSVN_Firmware:createDir $(ALL_OBJ)
	@echo 
	@echo "------------------------------------------------------------------"
	@echo "==== Linking $@ === "
	@echo "------------------------------------------------------------------"
	$(CXX) $(CFLAGS) $(ALL_OBJ) -L$(COMMAPPLIB) -L$(COMMPATH) -L$(COMMREL) -L$(AUTHREL) -L$(BASICLIB) $(APPLINK) $(CGILIBLINK)  $(BASICLIBLINK) -o $@ 
	@echo 
	@echo "------------------------------------------------------------------"
	@echo "Install [$@] to [$(INSTALL_PATH)]"
	$(MOVEFILE) new_app_GetSVN_Firmware $(INSTALL_PATH)
	
	
$(BUILD_CACHE_DIR)/%.o:%.cpp
	$(CXX) $(THIS_COMPILE) $< -o $@
	

clean:
	-rm -rf $(BUILD_CACHE_DIR)
