CXXFLAGS	+= -I./source/app/zigbee_manager

VPATH += source/app/zigbee_manager

OBJ += $(OBJ_DIR)/if_znp.o
OBJ += $(OBJ_DIR)/zcl.o
OBJ += $(OBJ_DIR)/znp_serial.o
