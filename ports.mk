# This file contains port specific Makefile rules. It is automatically
# included by the default (main) Makefile.
#

#
# POSIX specific
#
install:
	$(INSTALL) -d "$(DESTDIR)$(bindir)"
	$(INSTALL) -c -m 755 "./$(EXECUTABLE)" "$(DESTDIR)$(bindir)/$(EXECUTABLE)"
	$(INSTALL) -d "$(DESTDIR)$(mandir)/man6/"
	$(INSTALL) -c -m 644 "$(srcdir)/dists/cabalexec.6" "$(DESTDIR)$(mandir)/man6/cabalexec.6"
	$(INSTALL) -d "$(DESTDIR)$(datarootdir)/pixmaps/"
	$(INSTALL) -c -m 644 "$(srcdir)/icons/cabalexec.xpm" "$(DESTDIR)$(datarootdir)/pixmaps/cabalexec.xpm"
	$(INSTALL) -d "$(DESTDIR)$(datarootdir)/icons/hicolor/scalable/apps/"
	$(INSTALL) -c -m 644 "$(srcdir)/icons/cabalexec.svg" "$(DESTDIR)$(datarootdir)/icons/hicolor/scalable/apps/cabalexec.svg"
	$(INSTALL) -d "$(DESTDIR)$(docdir)"
	$(INSTALL) -c -m 644 $(DIST_FILES_DOCS) "$(DESTDIR)$(docdir)"
	$(INSTALL) -d "$(DESTDIR)$(datadir)"
	$(INSTALL) -c -m 644 $(DIST_FILES_THEMES) $(DIST_FILES_ENGINEDATA) "$(DESTDIR)$(datadir)/"
ifdef DYNAMIC_MODULES
	$(INSTALL) -d "$(DESTDIR)$(libdir)/cabalexec/"
	$(INSTALL) -c -m 644 $(PLUGINS) "$(DESTDIR)$(libdir)/cabalexec/"
endif

install-strip:
	$(INSTALL) -d "$(DESTDIR)$(bindir)"
	$(INSTALL) -c -s -m 755 "./$(EXECUTABLE)" "$(DESTDIR)$(bindir)/$(EXECUTABLE)"
	$(INSTALL) -d "$(DESTDIR)$(mandir)/man6/"
	$(INSTALL) -c -m 644 "$(srcdir)/dists/cabalexec.6" "$(DESTDIR)$(mandir)/man6/cabalexec.6"
	$(INSTALL) -d "$(DESTDIR)$(datarootdir)/pixmaps/"
	$(INSTALL) -c -m 644 "$(srcdir)/icons/cabalexec.xpm" "$(DESTDIR)$(datarootdir)/pixmaps/cabalexec.xpm"
	$(INSTALL) -d "$(DESTDIR)$(datarootdir)/icons/hicolor/scalable/apps/"
	$(INSTALL) -c -m 644 "$(srcdir)/icons/cabalexec.svg" "$(DESTDIR)$(datarootdir)/icons/hicolor/scalable/apps/cabalexec.svg"
	$(INSTALL) -d "$(DESTDIR)$(docdir)"
	$(INSTALL) -c -m 644 $(DIST_FILES_DOCS) "$(DESTDIR)$(docdir)"
	$(INSTALL) -d "$(DESTDIR)$(datadir)"
	$(INSTALL) -c -m 644 $(DIST_FILES_THEMES) $(DIST_FILES_ENGINEDATA) "$(DESTDIR)$(datadir)/"
ifdef DYNAMIC_MODULES
	$(INSTALL) -d "$(DESTDIR)$(libdir)/cabalexec/"
	$(INSTALL) -c -s -m 644 $(PLUGINS) "$(DESTDIR)$(libdir)/cabalexec/"
endif

uninstall:
	rm -f "$(DESTDIR)$(bindir)/$(EXECUTABLE)"
	rm -f "$(DESTDIR)$(mandir)/man6/cabalexec.6"
	rm -f "$(DESTDIR)$(datarootdir)/pixmaps/cabalexec.xpm"
	rm -f "$(DESTDIR)$(datarootdir)/icons/hicolor/scalable/apps/cabalexec.svg"
	rm -rf "$(DESTDIR)$(docdir)"
	rm -rf "$(DESTDIR)$(datadir)"
ifdef DYNAMIC_MODULES
	rm -rf "$(DESTDIR)$(libdir)/cabalexec/"
endif

# Special target to create a application wrapper for Mac OS X
bundle_name = Cabal.app
bundle: cabalexec-static
	mkdir -p $(bundle_name)/Contents/MacOS
	mkdir -p $(bundle_name)/Contents/Resources
	echo "APPL????" > $(bundle_name)/Contents/PkgInfo
	cp $(srcdir)/dists/macosx/Info.plist $(bundle_name)/Contents/
ifdef USE_SPARKLE
	mkdir -p $(bundle_name)/Contents/Frameworks
	cp $(srcdir)/dists/macosx/dsa_pub.pem $(bundle_name)/Contents/Resources/
	cp -R $(STATICLIBPATH)/Sparkle.framework $(bundle_name)/Contents/Frameworks/
endif
	cp $(srcdir)/icons/cabalexec.icns $(bundle_name)/Contents/Resources/
	cp $(DIST_FILES_DOCS) $(bundle_name)/
	cp $(DIST_FILES_THEMES) $(bundle_name)/Contents/Resources/
ifdef DIST_FILES_ENGINEDATA
	cp $(DIST_FILES_ENGINEDATA) $(bundle_name)/Contents/Resources/
endif
	$(srcdir)/devtools/credits.pl --rtf > $(bundle_name)/Contents/Resources/Credits.rtf
	chmod 644 $(bundle_name)/Contents/Resources/*
	cp cabalexec-static $(bundle_name)/Contents/MacOS/cabalexec
	chmod 755 $(bundle_name)/Contents/MacOS/cabalexec
	$(STRIP) $(bundle_name)/Contents/MacOS/cabalexec

iphonebundle: iphone
	mkdir -p $(bundle_name)
	cp $(srcdir)/dists/iphone/Info.plist $(bundle_name)/
	cp $(DIST_FILES_DOCS) $(bundle_name)/
	cp $(DIST_FILES_THEMES) $(bundle_name)/
ifdef DIST_FILES_ENGINEDATA
	cp $(DIST_FILES_ENGINEDATA) $(bundle_name)/
endif
	$(STRIP) cabalexec
	ldid -S cabalexec
	chmod 755 cabalexec
	cp cabalexec $(bundle_name)/Cabal
	cp $(srcdir)/dists/iphone/icon.png $(bundle_name)/
	cp $(srcdir)/dists/iphone/icon-72.png $(bundle_name)/
	cp $(srcdir)/dists/iphone/Default.png $(bundle_name)/

# Location of static libs for the iPhone
ifneq ($(BACKEND), iphone)
# Static libaries, used for the cabalexec-static and iphone targets
OSX_STATIC_LIBS := `$(SDLCONFIG) --static-libs`
endif

ifdef USE_FREETYPE2
OSX_STATIC_LIBS += $(STATICLIBPATH)/lib/libfreetype.a $(STATICLIBPATH)/lib/libbz2.a
endif

ifdef USE_VORBIS
OSX_STATIC_LIBS += \
		$(STATICLIBPATH)/lib/libvorbisfile.a \
		$(STATICLIBPATH)/lib/libvorbis.a \
		$(STATICLIBPATH)/lib/libogg.a
endif

ifdef USE_TREMOR
OSX_STATIC_LIBS += $(STATICLIBPATH)/lib/libvorbisidec.a
endif

ifdef USE_FLAC
OSX_STATIC_LIBS += $(STATICLIBPATH)/lib/libFLAC.a
endif

ifdef USE_FLUIDSYNTH
OSX_STATIC_LIBS += \
                -framework CoreAudio \
                $(STATICLIBPATH)/lib/libfluidsynth.a
endif

ifdef USE_MAD
OSX_STATIC_LIBS += $(STATICLIBPATH)/lib/libmad.a
endif

ifdef USE_PNG
OSX_STATIC_LIBS += $(STATICLIBPATH)/lib/libpng.a
endif

ifdef USE_THEORADEC
OSX_STATIC_LIBS += $(STATICLIBPATH)/lib/libtheoradec.a
endif

ifdef USE_FAAD
OSX_STATIC_LIBS += $(STATICLIBPATH)/lib/libfaad.a
endif

ifdef USE_MPEG2
OSX_STATIC_LIBS += $(STATICLIBPATH)/lib/libmpeg2.a
endif

ifdef USE_JPEG
OSX_STATIC_LIBS += $(STATICLIBPATH)/lib/libjpeg.a
endif

ifdef USE_ZLIB
OSX_ZLIB ?= $(STATICLIBPATH)/lib/libz.a
endif

ifdef USE_SPARKLE
OSX_STATIC_LIBS += -framework Sparkle -F$(STATICLIBPATH)
endif

ifdef USE_ICONV
OSX_ICONV ?= -liconv
endif

# Special target to create a static linked binary for Mac OS X.
# We use -force_cpusubtype_ALL to ensure the binary runs on every
# PowerPC machine.
cabalexec-static: $(OBJS)
	$(CXX) $(LDFLAGS) -force_cpusubtype_ALL -o cabalexec-static $(OBJS) \
		-framework CoreMIDI \
		$(OSX_STATIC_LIBS) \
		$(OSX_ZLIB) \
		$(OSX_ICONV)

# Special target to create a static linked binary for the iPhone
iphone: $(OBJS)
	$(CXX) $(LDFLAGS) -o cabalexec $(OBJS) \
		$(OSX_STATIC_LIBS) \
		-framework UIKit -framework CoreGraphics -framework OpenGLES \
		-framework CoreFoundation -framework QuartzCore -framework Foundation \
		-framework AudioToolbox -framework CoreAudio -lobjc -lz

# Special target to create a snapshot disk image for Mac OS X
# TODO: Replace AUTHORS by Credits.rtf
osxsnap: bundle
	mkdir Cabal-snapshot
	$(srcdir)/devtools/credits.pl --text > $(srcdir)/AUTHORS
	cp $(srcdir)/AUTHORS ./Cabal-snapshot/Authors
	cp $(srcdir)/COPYING ./Cabal-snapshot/License\ \(GPL\)
	cp $(srcdir)/COPYING.BSD ./Cabal-snapshot/License\ \(BSD\)
	cp $(srcdir)/COPYING.LGPL ./Cabal-snapshot/License\ \(LGPL\)
	cp $(srcdir)/COPYING.FREEFONT ./Cabal-snapshot/License\ \(FREEFONT\)
	cp $(srcdir)/COPYRIGHT ./Cabal-snapshot/Copyright\ Holders
	cp $(srcdir)/NEWS ./Cabal-snapshot/News
	cp $(srcdir)/README ./Cabal-snapshot/Cabal\ ReadMe
	mkdir Cabal-snapshot/doc
	cp $(srcdir)/doc/QuickStart ./Cabal-snapshot/doc/QuickStart
	mkdir Cabal-snapshot/doc/cz
	cp $(srcdir)/doc/cz/PrectiMe ./Cabal-snapshot/doc/cz/PrectiMe
	mkdir Cabal-snapshot/doc/da
	cp $(srcdir)/doc/da/HurtigStart ./Cabal-snapshot/doc/da/HurtigStart
	mkdir Cabal-snapshot/doc/de
	cp $(srcdir)/doc/de/Liesmich ./Cabal-snapshot/doc/de/Liesmich
	cp $(srcdir)/doc/de/Schnellstart ./Cabal-snapshot/doc/de/Schnellstart
	mkdir Cabal-snapshot/doc/es
	cp $(srcdir)/doc/es/InicioRapido ./Cabal-snapshot/doc/es
	mkdir Cabal-snapshot/doc/fr
	cp $(srcdir)/doc/fr/DemarrageRapide ./Cabal-snapshot/doc/fr/DemarrageRapide
	mkdir Cabal-snapshot/doc/it
	cp $(srcdir)/doc/it/GuidaRapida ./Cabal-snapshot/doc/it/GuidaRapida
	mkdir Cabal-snapshot/doc/no-nb
	cp $(srcdir)/doc/no-nb/HurtigStart ./Cabal-snapshot/doc/no-nb/HurtigStart
	mkdir Cabal-snapshot/doc/se
	cp $(srcdir)/doc/se/LasMig ./Cabal-snapshot/doc/se/LasMig
	cp $(srcdir)/doc/se/Snabbstart ./Cabal-snapshot/doc/se/Snabbstart
	/Developer/Tools/SetFile -t ttro -c ttxt ./Cabal-snapshot/*
	xattr -w "com.apple.TextEncoding" "utf-8;134217984" ./Cabal-snapshot/doc/cz/*
	xattr -w "com.apple.TextEncoding" "utf-8;134217984" ./Cabal-snapshot/doc/da/*
	xattr -w "com.apple.TextEncoding" "utf-8;134217984" ./Cabal-snapshot/doc/de/*
	xattr -w "com.apple.TextEncoding" "utf-8;134217984" ./Cabal-snapshot/doc/es/*
	xattr -w "com.apple.TextEncoding" "utf-8;134217984" ./Cabal-snapshot/doc/fr/*
	xattr -w "com.apple.TextEncoding" "utf-8;134217984" ./Cabal-snapshot/doc/it/*
	xattr -w "com.apple.TextEncoding" "utf-8;134217984" ./Cabal-snapshot/doc/no-nb/*
	xattr -w "com.apple.TextEncoding" "utf-8;134217984" ./Cabal-snapshot/doc/se/*
	/Developer/Tools/CpMac -r $(bundle_name) ./Cabal-snapshot/
	cp $(srcdir)/dists/macosx/DS_Store ./Cabal-snapshot/.DS_Store
	cp $(srcdir)/dists/macosx/background.jpg ./Cabal-snapshot/background.jpg
	/Developer/Tools/SetFile -a V ./Cabal-snapshot/.DS_Store
	/Developer/Tools/SetFile -a V ./Cabal-snapshot/background.jpg
	hdiutil create -ov -format UDZO -imagekey zlib-level=9 -fs HFS+ \
					-srcfolder Cabal-snapshot \
					-volname "Cabal" \
					Cabal-snapshot.dmg
	rm -rf Cabal-snapshot

#
# Windows specific
#

cabalexecwinres.o: $(srcdir)/icons/cabalexec.ico $(DIST_FILES_THEMES) $(DIST_FILES_ENGINEDATA) $(srcdir)/dists/cabalexec.rc
	$(QUIET_WINDRES)$(WINDRES) -DHAVE_CONFIG_H $(WINDRESFLAGS) $(DEFINES) -I. -I$(srcdir) $(srcdir)/dists/cabalexec.rc cabalexecwinres.o

# Special target to create a win32 snapshot binary (for Inno Setup)
win32dist: $(EXECUTABLE)
	mkdir -p $(WIN32PATH)
	mkdir -p $(WIN32PATH)/graphics
	mkdir -p $(WIN32PATH)/doc
	mkdir -p $(WIN32PATH)/doc/cz
	mkdir -p $(WIN32PATH)/doc/da
	mkdir -p $(WIN32PATH)/doc/de
	mkdir -p $(WIN32PATH)/doc/es
	mkdir -p $(WIN32PATH)/doc/fr
	mkdir -p $(WIN32PATH)/doc/it
	mkdir -p $(WIN32PATH)/doc/no-nb
	mkdir -p $(WIN32PATH)/doc/se
	$(STRIP) $(EXECUTABLE) -o $(WIN32PATH)/$(EXECUTABLE)
	cp $(srcdir)/AUTHORS $(WIN32PATH)/AUTHORS.txt
	cp $(srcdir)/COPYING $(WIN32PATH)/COPYING.txt
	cp $(srcdir)/COPYING.BSD $(WIN32PATH)/COPYING.BSD.txt
	cp $(srcdir)/COPYING.LGPL $(WIN32PATH)/COPYING.LGPL.txt
	cp $(srcdir)/COPYING.FREEFONT $(WIN32PATH)/COPYING.FREEFONT.txt
	cp $(srcdir)/COPYRIGHT $(WIN32PATH)/COPYRIGHT.txt
	cp $(srcdir)/NEWS $(WIN32PATH)/NEWS.txt
	cp $(srcdir)/doc/cz/PrectiMe $(WIN32PATH)/doc/cz/PrectiMe.txt
	cp $(srcdir)/doc/de/Neues $(WIN32PATH)/doc/de/Neues.txt
	cp $(srcdir)/doc/QuickStart $(WIN32PATH)/doc/QuickStart.txt
	cp $(srcdir)/doc/es/InicioRapido $(WIN32PATH)/doc/es/InicioRapido.txt
	cp $(srcdir)/doc/fr/DemarrageRapide $(WIN32PATH)/doc/fr/DemarrageRapide.txt
	cp $(srcdir)/doc/it/GuidaRapida $(WIN32PATH)/doc/it/GuidaRapida.txt
	cp $(srcdir)/doc/no-nb/HurtigStart $(WIN32PATH)/doc/no-nb/HurtigStart.txt
	cp $(srcdir)/doc/da/HurtigStart $(WIN32PATH)/doc/da/HurtigStart.txt
	cp $(srcdir)/doc/de/Schnellstart $(WIN32PATH)/doc/de/Schnellstart.txt
	cp $(srcdir)/doc/se/Snabbstart $(WIN32PATH)/doc/se/Snabbstart.txt
	cp $(srcdir)/README $(WIN32PATH)/README.txt
	cp $(srcdir)/doc/de/Liesmich $(WIN32PATH)/doc/de/Liesmich.txt
	cp $(srcdir)/doc/se/LasMig $(WIN32PATH)/doc/se/LasMig.txt
	cp /usr/local/README-SDL.txt $(WIN32PATH)
	cp /usr/local/bin/SDL.dll $(WIN32PATH)
	cp $(srcdir)/dists/win32/graphics/left.bmp $(WIN32PATH)/graphics
	cp $(srcdir)/dists/win32/graphics/cabalexec-install.ico $(WIN32PATH)/graphics
	cp $(srcdir)/dists/win32/migration.bat $(WIN32PATH)
	cp $(srcdir)/dists/win32/migration.txt $(WIN32PATH)
	cp $(srcdir)/dists/win32/Cabal.iss $(WIN32PATH)
	unix2dos $(WIN32PATH)/*.txt
	unix2dos $(WIN32PATH)/doc/*.txt
	unix2dos $(WIN32PATH)/doc/cz/*.txt
	unix2dos $(WIN32PATH)/doc/da/*.txt
	unix2dos $(WIN32PATH)/doc/de/*.txt
	unix2dos $(WIN32PATH)/doc/es/*.txt
	unix2dos $(WIN32PATH)/doc/fr/*.txt
	unix2dos $(WIN32PATH)/doc/it/*.txt
	unix2dos $(WIN32PATH)/doc/no-nb/*.txt
	unix2dos $(WIN32PATH)/doc/se/*.txt

# Special target to create a win32 NSIS installer
win32setup: $(EXECUTABLE)
	mkdir -p $(srcdir)/$(STAGINGPATH)
	$(STRIP) $(EXECUTABLE) -o $(srcdir)/$(STAGINGPATH)/$(EXECUTABLE)
	cp /usr/local/bin/SDL.dll $(srcdir)/$(STAGINGPATH)
	makensis -V2 -Dtop_srcdir="../.." -Dstaging_dir="../../$(STAGINGPATH)" -Darch=$(ARCH) $(srcdir)/dists/win32/cabalexec.nsi


#
# Special target to generate project files for various IDEs
# Mainly Win32-specific
#

# The release branch is in form 'heads/branch-1-4-1', for this case
# $CUR_BRANCH will be equal to '1', for the rest cases it will be empty
CUR_BRANCH := $(shell cd $(srcdir); git describe --all |cut -d '-' -f 4-)

ideprojects: devtools/create_project
ifeq ($(VER_DIRTY), -dirty)
	$(error You have uncommitted changes) 
endif 
ifeq "$(CUR_BRANCH)" "heads/master"
	$(error You cannot do it on master) 
else ifeq "$(CUR_BRANCH)" ""
	$(error You must be on a release branch) 
endif
	@echo Creating Code::Blocks project files...
	@cd $(srcdir)/dists/codeblocks && ../../devtools/create_project/create_project ../.. --codeblocks >/dev/null && git add -f engines/plugins_table.h *.workspace *.cbp
	@echo Creating MSVC9 project files...
	@cd $(srcdir)/dists/msvc9 && ../../devtools/create_project/create_project ../.. --msvc --msvc-version 9 >/dev/null && git add -f engines/plugins_table.h *.sln *.vcproj *.vsprops
	@echo Creating MSVC10 project files...
	@cd $(srcdir)/dists/msvc10 && ../../devtools/create_project/create_project ../.. --msvc --msvc-version 10 >/dev/null && git add -f engines/plugins_table.h *.sln *.vcxproj *.vcxproj.filters *.props
	@echo Creating MSVC11 project files...
	@cd $(srcdir)/dists/msvc11 && ../../devtools/create_project/create_project ../.. --msvc --msvc-version 11 >/dev/null && git add -f engines/plugins_table.h *.sln *.vcxproj *.vcxproj.filters *.props
	@echo Creating MSVC12 project files...
	@cd $(srcdir)/dists/msvc12 && ../../devtools/create_project/create_project ../.. --msvc --msvc-version 12 >/dev/null && git add -f engines/plugins_table.h *.sln *.vcxproj *.vcxproj.filters *.props
	@echo
	@echo All is done.
	@echo Now run
	@echo "\tgit commit -m 'DISTS: Generated Code::Blocks and MSVC project files'"

# Target to create Raspberry Pi zip containig binary and specific README
raspberrypi_dist:
	mkdir -p $(srcdir)/cabalexec-rpi
	cp $(srcdir)/backends/platform/sdl/raspberrypi/README.RASPBERRYPI $(srcdir)/cabalexec-rpi/README
	cp $(srcdir)/cabalexec $(srcdir)/cabalexec-rpi
	zip -r cabalexec-rpi.zip cabalexec-rpi
	rm -f -R cabalexec-rpi

# Mark special targets as phony
.PHONY: deb bundle osxsnap win32dist install uninstall
