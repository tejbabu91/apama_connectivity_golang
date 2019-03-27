#
# Makefile
# 
# GNU Make compatible makefile for the example plugins.
#
# $Revision: 273449 $ $Date: 2016-01-19 13:06:30 +0000 (Tue, 19 Jan 2016) $
#
# $Copyright (c) 2016-2017 Software AG, Darmstadt, Germany and/or Software AG USA Inc., Reston, VA, USA, and/or its subsidiaries and/or its affiliates and/or their licensors.$
# Use, reproduction, transfer, publication or disclosure is prohibited except as specifically provided for in your License Agreement with Software AG
# 


# ===========================================================================
# Apama installation root -- modify or override on command line as necessary
# ===========================================================================

ifndef APAMA_HOME
home_not_set:
	@echo Please set the APAMA_HOME environment variable
endif


# What OS are we running?
OS := $(shell uname)


ifeq ($(OS), Linux)

CXX := /usr/bin/g++


DEBUG_FLAGS := -ggdb -Wall -Werror -pedantic
#DEBUG_FLAGS := -O3 -Wall -Werror -pedantic
CXXFLAGS := -fPIC -std=c++11
LDFLAGS = -shared -Wl,-soname=$(@F)
LIBS :=
DEFINES := -D__linux__ -D__unix__ -D__OSVERSION__=2 -D__STDC_FORMAT_MACROS -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS

endif  # Linux

ARCH := $(shell uname -m)

ifeq ($(ARCH), x86_64)
DEFINES := $(DEFINES) -D__x86_64__ -D__i86__
endif # x86_64

ifeq ($(ARCH), armv7l)
DEFINES := $(DEFINES) -D__arm__ -mfloat-abi=hard
endif # armv7l


# Include paths
CPPFLAGS += -I. -I"$(APAMA_HOME)/include"

# Compiler flags
CXXFLAGS += $(DEBUG_FLAGS) $(DEFINES)

# Linker flags -- used when building executables
LDFLAGS += "-L$(APAMA_HOME)/lib" -lapclient



# =============================
# Plugin library build targets
# =============================

ALL := Release/libGoTransport.so

all: $(ALL)



# -----------------------
# Simple plugin, C++ API
# -----------------------
Release/libGoTransport.so: ./main.go ./user_go_layer/user.go
	mkdir -p Release
	CGO_CFLAGS="$(CFLAGS)" CGO_CPPFLAGS="$(CPPFLAGS)" CGO_CXXFLAGS="$(CXXFLAGS)" CGO_LDFLAGS="$(LDFLAGS)" go build -buildmode=c-shared -o $@



# ========
# Cleanup
# ========

RM := rm -rf

clean:
	-$(RM) Release
