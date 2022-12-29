# for sub directory
export SRC_AMSFILES :=
export SRC_C_FILES :=
export SRC_INCFILES :=
export SRC_CXX_FILES :=
export SRC_INCDIR :=
export LINK_FILES :=


######################################SUB_MK###################################################
SUBDIRS := $(shell find $(CURRENT_DIR) -maxdepth 4 -type d)
SUBMK += $(foreach dir, $(SUBDIRS), $(wildcard $(dir)/*.mk))
#$(info "$(SUBMK)")
include $(SUBMK)

################################################################################################
TARGET           	?= tonyserver
OUTPUTDIR := $(CURDIR)/output
###################################################COMPILE######################################
CROSS_COMPILE    	?= 

CC               	:= $(CROSS_COMPILE)gcc
CXX              	:= $(CROSS_COMPILE)g++
LD               	:= $(CROSS_COMPILE)ld
OBJCOPY          	:= $(CROSS_COMPILE)objcopy
OBJDUMP          	:= $(CROSS_COMPILE)objdump
SIZEINFO            := $(CROSS_COMPILE)size

##################################################COMPILE_FLAGS#################################
C_COMPILE_FLAGS 	:= -Wall
CXX_COMPILE_FLAGS 	:= -std=c++11 -g -Wall -O3 -D_PTHREADS
LFLAGS := -lpthread
#################################################################################################
DEFINE    :=
###############################################################
CFLAGS 				+=  $(C_COMPILE_FLAGS) $(DEFINE)
CXXFLAGS 			+=  $(CXX_COMPILE_FLAGS) $(DEFINE)

INCLUDE 		 := $(patsubst %, -I %, $(SRC_INCDIR))
CFILES			 := $(SRC_C_FILES)
CXXFILES         := $(SRC_CXX_FILES)

CFILENAME 		 := $(notdir $(CFILES))
CXXFILENAME		 := $(notdir $(CXXFILES))

COBJS		 	 := $(patsubst %, $(OUTPUTDIR)/%, $(CFILENAME:.c=.o))
CXXOBJS          := $(patsubst %, $(OUTPUTDIR)/%, $(CXXFILENAME:.cpp=.o))
OBJS			 := $(COBJS) $(CXXOBJS)

SRCDIRS          := $(dir $(CFILES)) $(dir $(CXXFILES))
VPATH			 := $(SRCDIRS)

.PHONY: clean


#$(info "SFILES = $(SFILES) ")
#$(info "CFILES = $(CFILES) ")

$(OUTPUTDIR)/$(TARGET).elf:$(OBJS)
	$(CXX) -o $(OUTPUTDIR)/$(TARGET).elf $^ $(LDFLAGS) $(LFLAGS)
#$(OBJCOPY) -O binary -S $(OUTPUTDIR)/$(TARGET).elf $(OUTPUTDIR)/$(TARGET).bin
#$(OBJDUMP) -D -m arm $(OUTPUTDIR)/$(TARGET).elf > $(OUTPUTDIR)/$(TARGET).dis
	$(SIZEINFO) $@

$(COBJS) : $(OUTPUTDIR)/%.o : %.c
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<

$(CXXOBJS) : $(OUTPUTDIR)/%.o : %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCLUDE) -o $@ $<

clean:
	rm -rf $(OUTPUTDIR)/*
