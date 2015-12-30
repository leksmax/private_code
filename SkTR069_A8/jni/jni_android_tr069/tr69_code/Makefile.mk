#############################################################
#
#	Copyright (c) 2009-2011, ShenzhenSkyworth Digital Technology Co., Ltd
#	All rights reserved.
#
#	File name: Makefile.mk
#	Description: define makefile rulse for project EU7
#	Version: 1.0
#	Author: Deng Yu (dengyu@skyworth.com)
#	Date: 2010-06-21
#
##############################################################

PRJ_NAME = EU7

#COMPILE_FLAG = Release
#COMPILE_FLAG = Debug

##############################################################
# define compile commands
##############################################################
#CROSS_COMPILE = arm-vfp_uclibc-linux-gnu-
CROSS_COMPILE =arm-hisiv200-linux-


CC			= $(CROSS_COMPILE)gcc
CXX			= $(CROSS_COMPILE)g++
LD			= $(CROSS_COMPILE)ld
AS			= $(CROSS_COMPILE)as
AR			= $(CROSS_COMPILE)ar
NM			= $(CROSS_COMPILE)nm
STRIP		= ls
SSTRIP		= $(CROSS_COMPILE)strip
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump
RANLIB		= $(CROSS_COMPILE)ranlib

##############################################################
# define shell commands
##############################################################
MAKE		= make
CP			= cp -rf
RM			= rm -rf
SORT		= sort
SED			= sed
TOUCH		= touch
MKDIR		= mkdir -p
TAR         = tar

##############################################################
# define output files path
##############################################################
OUT_BIN_FOLDER		= $(PRJ_ROOT)/bin
OUT_LIB_FOLDER		= $(PRJ_ROOT)/lib
OUT_THIRD_LIBS_FOLDER	= $(PRJ_ROOT)/third_libs

##############################################################
# define compile variables
##############################################################
INCLUDE_PATH = ../event/include  ../event/include/event2
LIB_PATH = ../event/.lib 
DEBUG_FLAG = -O0 -g -Wall -I. -shared -fPIC -c
#RELEASE_FLAG = -O3 -Wall -I. -shared -fPIC -DNDEBUG -c
RELEASE_FLAG = -O3 -Wall -I. -shared -fPIC -c

ifeq ($(DEBUG), y)
CFLAGS := $(DEBUG_FLAG)
CFLAGS += -DSK_DEBUG
else
CFLAGS := $(RELEASE_FLAG)
endif

ifeq ($(PRODUCT_VERSION), JS-CTC)
CFLAGS += -DSK_JS_CTC
endif

ifeq ($(PRODUCT_VERSION), HN-CTC)
CFLAGS += -DSK_HN_CTC
endif

ifeq ($(PRODUCT_VERSION), JX-CTC)
CFLAGS += -DSK_JX_CTC
endif

ifeq ($(PRODUCT_VERSION), SH-CTC)
CFLAGS += -DSK_SH_CTC
endif

ifeq ($(PRODUCT_VERSION), HB-CTC)
CFLAGS += -DSK_HB_CTC
endif

ifeq ($(PRODUCT_VERSION), SC-CTC)
CFLAGS += -DSK_SC_CTC
endif

ifeq ($(PRODUCT_VERSION), ZJ-CTC)
CFLAGS += -DSK_ZJ_CTC
endif

ifeq ($(PRODUCT_VERSION), FJ-CTC)
CFLAGS += -DSK_FJ_CTC
endif


##支持高清平台
ifeq ($(HD_PLATFORM), y)    
CFLAGS += -DHD_PLATFORM
endif




INCLUDE = $(addprefix -I, $(INCLUDE_PATH))
LD_FLAGS = $(addprefix -L, $(LIB_PATH)) -Wl,-Bdynamic -lpthread

CURRENT_DIR			:=  $(shell cd ./; pwd)
#C_SRCS				=  $(wildcard $(CURRENT_DIR)/*.c)
#CPP_SRCS			=  $(wildcard $(CURRENT_DIR)/*.cpp)
C_SRCS				=  $(notdir $(wildcard $(CURRENT_DIR)/*.c))
CPP_SRCS			= $(notdir $(wildcard $(CURRENT_DIR)/*.cpp))
C_OBJS				=  $(C_SRCS:.c=.o)
CPP_OBJS			=  $(CPP_SRCS:.cpp=.o)
OBJS				=  $(C_OBJS) $(CPP_OBJS)
DEPS				=  $(OBJS:.o=.d)
#MISSING_DEPS		:= $(filter-out $(wildcard $(DEPS)),$(DEPS))
#MISSING_DEPS_SOURCES := $(wildcard $(patsubst %.d,%.c,$(MISSING_DEPS)) \
#					    $(patsubst %.d,%.cc,$(MISSING_DEPS)))
