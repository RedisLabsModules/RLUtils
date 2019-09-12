
ROOT=.
include $(ROOT)/deps/readies/mk/main

BINDIR=$(BINROOT)/src
SRCDIR=src

RMUTILS_PRFX=example

#----------------------------------------------------------------------------------------------

TARGET=$(BINROOT)/rlutils.a
EXAMPLE=$(BINROOT)/example.so

CC=gcc

CC_FLAGS = \
	-DRMUTILS_PRFX=$(RMUTILS_PRFX) \
	-Wall \
	-fPIC \
	-std=gnu99 \
	-MMD \
	-MF $(@:.o=.d)

LD_FLAGS += 
LD_LIBS += -lc -lm -lpthread -ldl

ifeq ($(OS),linux)
SO_LD_FLAGS += -shared -Bsymbolic $(LD_FLAGS)
endif

ifeq ($(OS),macosx)
SO_LD_FLAGS += -bundle -undefined dynamic_lookup -Wl,-macosx_version_min,10.8 $(LD_FLAGS)
endif

ifeq ($(DEBUG),1)
CC_FLAGS += -g -ggdb -O0
LD_FLAGS += -g
else
CC_FLAGS += -O3
endif

_SOURCES=\
	rlutils_config.c 		\
	rlutils_info.c   		\
	rlutils.c        		\
	utils/adlist.c   		\
	utils/buffer.c  		\
	utils/dict.c     		\
	memory/rlutils_memory.c

SOURCES=$(addprefix $(SRCDIR)/,$(_SOURCES))
OBJECTS=$(patsubst $(SRCDIR)/%.c,$(BINDIR)/%.o,$(SOURCES))

CC_DEPS = $(patsubst $(SRCDIR)/%.c, $(BINDIR)/%.d, $(SOURCES))

include $(MK)/defs

#----------------------------------------------------------------------------------------------

.PHONY: all build clean

build: $(TARGET) $(EXAMPLE)

include $(MK)/rules

clean:
ifeq ($(ALL),1)
	-$(SHOW)rm -rf $(BINROOT)
else
	-$(SHOW)[ -e $(BINDIR) ] && find $(BINDIR) -name '*.[oadh]' -type f -delete
	-rm $(EXAMPLE) $(TARGET)
endif # ALL

-include $(CC_DEPS)

$(BINDIR)/%.o: $(SRCDIR)/%.c
	@echo Compiling $<...
	$(SHOW)$(CC) $(CC_FLAGS) -c $< -o $@

$(TARGET): $(BIN_DIRS) $(OBJECTS)
	@echo Creating archive $@...
	$(SHOW)ar rs $@ $(OBJECTS)
	
$(EXAMPLE): $(SRCDIR)/example_module/example_module.c $(TARGET) 
	 @echo Building exmple module $@...
	 $(CC) -shared  $(LD_FLAGS) $(CC_FLAGS) -o $@ $< $(TARGET)

#---------------------------------------------------------------------------------------------- 