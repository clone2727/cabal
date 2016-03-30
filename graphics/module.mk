MODULE := graphics

MODULE_OBJS := \
	conversion.o \
	cursorman.o \
	font.o \
	fontman.o \
	fonts/bdf.o \
	fonts/consolefont.o \
	fonts/newfont_big.o \
	fonts/newfont.o \
	fonts/ttf.o \
	fonts/winfont.o \
	maccursor.o \
	pixelformat.o \
	primitives.o \
	scaler.o \
	scaler/thumbnail_intern.o \
	sjis.o \
	surface.o \
	transform_struct.o \
	transform_tools.o \
	transparent_surface.o \
	thumbnail.o \
	VectorRenderer.o \
	VectorRendererSpec.o \
	wincursor.o \
	yuv_to_rgb.o

ifdef USE_SCALERS
MODULE_OBJS += \
	scaler/2xsai.o \
	scaler/aspect.o \
	scaler/downscaler.o \
	scaler/scale2x.o \
	scaler/scale3x.o \
	scaler/scalebit.o

ifdef USE_HQ_SCALERS
MODULE_OBJS += \
	scaler/hq2x.o \
	scaler/hq3x.o

endif

endif

# Include common rules
include $(srcdir)/rules.mk
