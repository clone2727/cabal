MODULE := engines/mystjag

MODULE_OBJS = \
	detection.o \
	mystjag.o


# This module can be built as a plugin
ifeq ($(ENABLE_MYSTJAG), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
