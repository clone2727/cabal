MODULE := backends/platform/android

MODULE_OBJS := \
	jni.o \
	texture.o \
	asset-archive.o \
	android.o \
	gfx.o \
	events.o

ifdef USE_FREETYPE2
MODULE_OBJS += \
	android-font-provider.o
endif

# We don't use rules.mk but rather manually update OBJS and MODULE_DIRS.
MODULE_OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS))
OBJS := $(MODULE_OBJS) $(OBJS)
MODULE_DIRS += $(sort $(dir $(MODULE_OBJS)))
