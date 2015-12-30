############################################################################
# MODULE : skyapi_util                																		 #
# Filename:     makefile                                                   #
# Version:	 1.0.0 																												 #
# Copyright (c) 2008, SkyworthDTV                                          #
# All rights reserved.                                                     #
# Modification History:                                                    #
# Date	     Name 	Description                                            #
# 2009-4-29	 skyworth   	Created                                          #
############################################################################

ifdef SK_IN_OBJECT_DIR

################################################
### skyworth cfg
################################################
include $(SK_RULES)/sk_generic.mak

###########################################################
### settings
###########################################################
SK_TEST_ENABLE:=NO
###########################################################
### out put include file and include dir
###########################################################
OUT_INC_DIR = $(SK_RULES)/../include/skapi

###########################################################
### module
###########################################################
SK_MODULE_NAME:= sktr069
COMPILE_TARGET_TYPE := dll

SK_BUILD_DIR:=$(SK_CURR_DIR)
SK_MODULE_ROOT:=$(SK_CURR_MODULE_DIR)
SK_MODULE_EXPORT_DIR:=$(subst $(BAD_SLASH),$(GOOD_SLASH),$(SK_MODULE_ROOT)/../out)
SK_MODULE_LIB_NAME:=$(addsuffix .a,$(addprefix ${SK_MODULE_EXPORT_DIR}$(SLASH)lib,$(SK_MODULE_NAME)))

############################################################################
### path
############################################################################
vpath %.c ${SK_MODULE_ROOT}/

ifeq ($(SK_TEST_ENABLE),YES)
vpath %.c ${SK_MODULE_ROOT}/test
endif

CFLAGS += -I $(SK_MODULE_ROOT)
CFLAGS += -DHD_PLATFORM


############################################################################
### src
############################################################################

### api
SRCS:= $(notdir $(wildcard $(SK_CURR_MODULE_DIR)/*.c))

### test
ifeq ($(SK_TEST_ENABLE),YES)
SRCS += $(SK_MODULE_NAME)_test.c
endif

############################################################################
### build
############################################################################

LINUX_TARGETS := sktr069
	
TARGET:=${LINUX_TARGETS}


SK_SVN_VERSION_FILE_H:=$(subst $(BAD_SLASH),$(GOOD_SLASH),$(SK_MODULE_ROOT)/src/svn_version.h)
SK_SVN_FILE:=$(subst $(BAD_SLASH),$(GOOD_SLASH),$(SK_ROOT)/.svn/entries)
ifeq ($(SK_ENV),linux)
SK_SVN_VERSION_TOOL:=$(subst $(BAD_SLASH),$(GOOD_SLASH),$(SK_ROOT)/tool/linux/make_svn_version)
else
SK_SVN_VERSION_TOOL:=$(subst $(BAD_SLASH),$(GOOD_SLASH),$(SK_ROOT)/tool/windows/make_svn_version.exe)
endif

ifeq ($(SKY_SVN_INFO_SUPPORT),YES)
PREPROCESS:=$(SK_SVN_VERSION_FILE_H)
endif


include $(SK_RULES)/sk_rules.mak

$(SK_SVN_VERSION_FILE_H):$(SK_SVN_FILE)
	@echo "subdir clean" 
	$(SK_SVN_VERSION_TOOL) $(SK_SVN_FILE) $(SK_SVN_VERSION_FILE_H)

install:
	cp -u $(SK_CURR_DIR)/../sk_page_event.h $(OUT_INC_DIR)
	cp -u $(SK_CURR_DIR)/../sk_player_event.h $(OUT_INC_DIR)
	cp -u $(SK_CURR_DIR)/../sk_sound_event.h $(OUT_INC_DIR)

clean: subclean clean_destdir
	@$(RM) $(subst $(BAD_SLASH),$(GOOD_SLASH),$(SK_MODULE_EXPORT_DIR)/$(notdir $(SK_MODULE_LIB_NAME)))
test: 
	@echo "[Reconstruction...]" 	
else

include $(SK_RULES)/sk_builddir.mak

endif

############################################################################
### eof
############################################################################
