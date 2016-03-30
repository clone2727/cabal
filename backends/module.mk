MODULE := backends

MODULE_OBJS := \
	base-backend.o \
	modular-backend.o \
	audiocd/default/default-audiocd.o \
	events/default/default-events.o \
	fs/abstract-fs.o \
	fs/stdiostream.o \
	log/log.o \
	midi/alsa.o \
	midi/dmedia.o \
	midi/seq.o \
	midi/sndio.o \
	midi/stmidi.o \
	midi/timidity.o \
	saves/savefile.o \
	saves/default/default-saves.o \
	timer/default/default-timer.o


ifdef USE_ELF_LOADER
MODULE_OBJS += \
	plugins/elf/arm-loader.o \
	plugins/elf/elf-loader.o \
	plugins/elf/elf-provider.o \
	plugins/elf/memory-manager.o \
	plugins/elf/mips-loader.o \
	plugins/elf/ppc-loader.o \
	plugins/elf/shorts-segment-manager.o \
	plugins/elf/version.o
endif

ifdef ENABLE_KEYMAPPER
MODULE_OBJS += \
	keymapper/action.o \
	keymapper/hardware-input.o \
	keymapper/keymap.o \
	keymapper/keymapper.o \
	keymapper/remap-dialog.o
endif

ifdef ENABLE_VKEYBD
MODULE_OBJS += \
	vkeybd/image-map.o \
	vkeybd/polygon.o \
	vkeybd/virtual-keyboard.o \
	vkeybd/virtual-keyboard-gui.o \
	vkeybd/virtual-keyboard-parser.o
endif

# OpenGL specific source files.
ifdef USE_OPENGL
MODULE_OBJS += \
	graphics/opengl/debug.o \
	graphics/opengl/extensions.o \
	graphics/opengl/opengl-graphics.o \
	graphics/opengl/texture.o
endif

# SDL specific source files.
# We cannot just check $BACKEND = sdl, as various other backends
# derive from the SDL backend, and they all need the following files.
ifdef SDL_BACKEND
MODULE_OBJS += \
	events/sdl/sdl-events.o \
	graphics/sdl/sdl-graphics.o \
	graphics/surfacesdl/surfacesdl-graphics.o \
	mixer/doublebuffersdl/doublebuffersdl-mixer.o \
	mixer/sdl/sdl-mixer.o \
	mutex/sdl/sdl-mutex.o \
	plugins/sdl/sdl-provider.o \
	timer/sdl/sdl-timer.o

# SDL 2 removed audio CD support
ifndef USE_SDL2
MODULE_OBJS += \
	audiocd/sdl/sdl-audiocd.o
endif

ifdef USE_OPENGL
MODULE_OBJS += \
	graphics/openglsdl/openglsdl-graphics.o
endif
endif

ifdef POSIX
MODULE_OBJS += \
	fs/posix/posix-fs.o \
	fs/posix/posix-fs-factory.o \
	plugins/posix/posix-provider.o \
	saves/posix/posix-saves.o
endif

ifdef MACOSX
MODULE_OBJS += \
	midi/coreaudio.o \
	midi/coremidi.o
endif

ifdef WIN32
MODULE_OBJS += \
	fs/windows/windows-fs.o \
	fs/windows/windows-fs-factory.o \
	midi/windows.o \
	plugins/win32/win32-provider.o \
	saves/windows/windows-saves.o \
	taskbar/win32/win32-taskbar.o
endif

# Include common rules
include $(srcdir)/rules.mk
