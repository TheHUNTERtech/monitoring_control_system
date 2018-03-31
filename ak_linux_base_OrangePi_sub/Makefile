###############################################################################
# author: ThanNT
# date: 6/1/2017
###############################################################################
RPI_SYS_DIR	=
TOOLS_DIR	=
NAME_MODULE	= ak_linux_base
CXX		= g++
CC		= gcc
OBJ_DIR		= build_$(NAME_MODULE)

OPTIMIZE_OPTION	=	-g -Os
#WARNNING_OPTION	+=	-Werror -W -Wno-missing-field-initializers

-include source/ak/Makefile.mk
-include source/sys/Makefile.mk
-include source/app/Makefile.mk
-include source/common/Makefile.mk
-include source/driver/Makefile.mk

CXXFLAGS	+= -I/usr/local/include
CXXFLAGS	+= -I/usr/include

# CXX compiler option
CXXFLAGS	+=	$(OPTIMIZE_OPTION)	\
		$(WARNNING_OPTION)	\
		-std=c++11		\
		-Wall			\
		-Winline		\
		-pipe			\
		-g			\

# Library paths
LDFLAGS	+= -L/usr/local/lib
LDFLAGS	+= -L/usr/include
LDFLAGS	+= -Wl,-Map=$(OBJ_DIR)/$(NAME_MODULE).map

#Library libs
LDLIBS	+=	-lpthread		\
                -lmosquitto		\
                -lmosquittopp		\
                -lrt			\
                -lm			\
                -lcrypto	\
                -lcurl			\

all: create $(OBJ_DIR)/$(NAME_MODULE)

create:
	@echo mkdir -p $(OBJ_DIR)
	@mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: %.cpp
	@echo CXX $<
	@$(CXX) -c -o $@ $< $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)

$(OBJ_DIR)/%.o: %.c
	@echo CXX $<
	@$(CC) -c -o $@ $< $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)

$(OBJ_DIR)/$(NAME_MODULE): $(OBJ)
	@echo ---------- START LINK PROJECT ----------
	@echo $(CXX) -o $@ $^ $(CXXFLAGS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)

copy:
	scp -r ./* root@192.168.1.5:/home/root/workspace/ak_linux_base

flash:
	@sudo LD_LIBRARY_PATH=/usr/local/lib/ $(OBJ_DIR)/$(NAME_MODULE)

debug:
	sudo gdb $(OBJ_DIR)/$(NAME_MODULE)

install:
	cp $(OBJ_DIR)/$(NAME_MODULE) /usr/local/bin

clean:
	@echo rm -rf $(OBJ_DIR)
	@rm -rf $(OBJ_DIR)
