MODULE := video

MODULE_OBJS := \
	avi_decoder.o \
	bink_decoder.o \
	coktel_decoder.o \
	dxa_decoder.o \
	flic_decoder.o \
	mpegps_decoder.o \
	psx_decoder.o \
	qt_decoder.o \
	smk_decoder.o \
	video_decoder.o

ifdef USE_THEORADEC
MODULE_OBJS += \
	ogg_decoder.o
endif

# Include common rules
include $(srcdir)/rules.mk
