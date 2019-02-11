# Makefile template for shared library

CC = gcc
CPPFLAGS = -fPIC -O3 -rdynamic
LDFLAGS = -shared
RM = rm -f
CP = cp -f


COALA_MODULES_DIR = /usr/lib/coala/modules
MAIN_DIR = $(CURDIR)
# MAIN_DIR = /usr/src/coala/gen/

FOLDERS := gen_dsp .
SRC_DIR := $(addprefix $(MAIN_DIR)/,$(FOLDERS))
SOURCES := $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.cpp))
INCLUDES = -I$(MAIN_DIR) -I$(MAIN_DIR)/gen_dsp
OBJS = $(SOURCES:.cpp=.o)

TARGET = $(MAIN_DIR)/libgenmodule.so

define cc-command
$(CC) $(CPPFLAGS) $< -o $@
endef

all: $(TARGET)
.PHONY: all

$(TARGET) : $(OBJS)
	$(CC) $(CPPFLAGS) $(OBJS) -o $@ $(LDFLAGS)
	$(CP) $(TARGET) $(COALA_MODULES_DIR)

.cpp.o:
	echo $(SRC_DIR)
	$(CC) $(CPPFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM) $(MAIN_DIR)/*.o $(MAIN_DIR)/*~ $(TARGET) $(MAIN_DIR)/gen_dsp/*.o $(MAIN_DIR)/gen_dsp/*~