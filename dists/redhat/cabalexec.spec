#------------------------------------------------------------------------------
#   cabalexec.spec
#       This SPEC file controls the building of Cabal RPM packages.
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#   Prologue information
#------------------------------------------------------------------------------
Name		: cabalexec
Version		: 1.0.0git
Release		: 1
Summary		: Legacy game interpreter
Group		: Interpreters
License		: GPL

Url             : http://project-cabal.org

Source		: %{name}-%{version}.tar.bz2
Source1		: libmad-0.15.1b.tar.bz2
Source2		: faad2-2.7.tar.bz2
Source3		: mpeg2dec-0.4.1.tar.bz2
Patch0		: libmad-0.15.1b-fixes-1.patch
BuildRoot	: %{_tmppath}/%{name}-%{version}-root

BuildRequires: desktop-file-utils
BuildRequires: libogg-devel
BuildRequires: libvorbis-devel
BuildRequires: flac-devel
BuildRequires: zlib-devel
BuildRequires: nasm
BuildRequires: SDL-devel >= 1.2.2
BuildRequires: freetype-devel

#------------------------------------------------------------------------------
#   Description
#------------------------------------------------------------------------------
%description
Cabal is an interpreter that will play many legacy games,
including LucasArts SCUMM games (such as Monkey Island 1-3, Day of the
Tentacle, Sam & Max, ...), many of Sierra's AGI and SCI games (such as King's
Quest 1-6, Space Quest 1-5, ...), Discworld 1 and 2, Simon the Sorcerer 1 and
2, Beneath A Steel Sky, Lure of the Temptress, Broken Sword 1 and 2, Flight of
the Amazon Queen, Gobliiins 1-3, The Legend of Kyrandia 1-3, many of Humongous
Entertainment's children's SCUMM games (including Freddi Fish and Putt Putt
games) and many more. See http://project-cabal.org for a full compatibility list.

#------------------------------------------------------------------------------
#   install scripts
#------------------------------------------------------------------------------
%prep
%setup -q -a 1 -a 2 -a 3 -n cabalexec-%{version}
%patch0 -p0
mkdir tmp

%build
(cd libmad-0.15.1b; ./configure --enable-static --disable-shared --prefix=%{_builddir}/cabalexec-%{version}/tmp; make; make install)
(cd faad2-2.7; ./configure --enable-static --disable-shared --prefix=%{_builddir}/cabalexec-%{version}/tmp; make; make install)
(cd mpeg2dec-0.4.1; ./configure --enable-static --disable-shared --prefix=%{_builddir}/cabalexec-%{version}/tmp; make; make install)
./configure --with-mad-prefix=%{_builddir}/cabalexec-%{version}/tmp --with-faad-prefix=%{_builddir}/cabalexec-%{version}/tmp --with-mpeg2-prefix=%{_builddir}/cabalexec-%{version}/tmp --prefix=%{_prefix} --enable-release
make

%install
install -m755 -D cabalexec %{buildroot}%{_bindir}/cabalexec
install -m644 -D dists/cabalexec.6 %{buildroot}%{_mandir}/man6/cabalexec.6
install -m644 -D icons/cabalexec.xpm %{buildroot}%{_datadir}/pixmaps/cabalexec.xpm
install -m644 -D icons/cabalexec.svg %{buildroot}%{_datadir}/icons/hicolor/scalable/apps/cabalexec.svg
install -m644 -D dists/redhat/cabalexec48.png %{buildroot}%{_datadir}/icons/hicolor/48x48/apps/cabalexec.png
install -m644 -D gui/themes/scummclassic.zip %{buildroot}%{_datadir}/cabalexec/scummclassic.zip
install -m644 -D gui/themes/scummmodern.zip %{buildroot}%{_datadir}/cabalexec/scummmodern.zip
install -m644 -D gui/themes/translations.dat %{buildroot}%{_datadir}/cabalexec/translations.dat
install -m644 -D dists/pred.dic %{buildroot}%{_datadir}/cabalexec/pred.dic
install -m644 -D dists/engine-data/kyra.dat %{buildroot}%{_datadir}/cabalexec/kyra.dat
install -m644 -D dists/engine-data/lure.dat %{buildroot}%{_datadir}/cabalexec/lure.dat
install -m644 -D dists/engine-data/queen.tbl %{buildroot}%{_datadir}/cabalexec/queen.tbl
install -m644 -D dists/engine-data/sky.cpt %{buildroot}%{_datadir}/cabalexec/sky.cpt
install -m644 -D dists/engine-data/drascula.dat %{buildroot}%{_datadir}/cabalexec/drascula.dat
install -m644 -D dists/engine-data/teenagent.dat %{buildroot}%{_datadir}/cabalexec/teenagent.dat
install -m644 -D dists/engine-data/hugo.dat %{buildroot}%{_datadir}/cabalexec/hugo.dat
install -m644 -D dists/engine-data/tony.dat %{buildroot}%{_datadir}/cabalexec/tony.dat
install -m644 -D dists/engine-data/toon.dat %{buildroot}%{_datadir}/cabalexec/toon.dat
install -m644 -D dists/engine-data/wintermute.zip %{buildroot}%{_datadir}/cabalexec/wintermute.zip
install -m644 -D dists/engine-data/neverhood.dat %{buildroot}%{_datadir}/cabalexec/neverhood.dat
desktop-file-install --vendor cabalexec --dir=%{buildroot}/%{_datadir}/applications dists/cabalexec.desktop

%clean
rm -Rf ${RPM_BUILD_ROOT}

%post
touch --no-create %{_datadir}/icons/hicolor || :
if [ -x %{_bindir}/gtk-update-icon-cache ]; then
        %{_bindir}/gtk-update-icon-cache --quiet %{_datadir}/icons/hicolor || :
fi

%postun
touch --no-create %{_datadir}/icons/hicolor || :
if [ -x %{_bindir}/gtk-update-icon-cache ]; then
        %{_bindir}/gtk-update-icon-cache --quiet %{_datadir}/icons/hicolor || :
fi

#------------------------------------------------------------------------------
#   Files listing.
#------------------------------------------------------------------------------
%files
%defattr(0644,root,root,0755)
%doc AUTHORS README NEWS COPYING COPYING.LGPL COPYING.FREEFONT COPYING.BSD COPYRIGHT
%attr(0755,root,root)%{_bindir}/cabalexec
%{_datadir}/applications/*
%{_datadir}/pixmaps/cabalexec.xpm
%{_datadir}/icons/hicolor/48x48/apps/cabalexec.png
%{_datadir}/icons/hicolor/scalable/apps/cabalexec.svg
%{_datadir}/cabalexec/scumm*.zip
%{_datadir}/cabalexec/translations.dat
%{_datadir}/cabalexec/pred.dic
%{_datadir}/cabalexec/kyra.dat
%{_datadir}/cabalexec/queen.tbl
%{_datadir}/cabalexec/sky.cpt
%{_datadir}/cabalexec/lure.dat
%{_datadir}/cabalexec/drascula.dat
%{_datadir}/cabalexec/teenagent.dat
%{_datadir}/cabalexec/hugo.dat
%{_datadir}/cabalexec/tony.dat
%{_datadir}/cabalexec/toon.dat
%{_datadir}/cabalexec/wintermute.zip
%{_datadir}/cabalexec/neverhood.dat
%{_mandir}/man6/cabalexec.6*

#------------------------------------------------------------------------------
#   Change Log
#------------------------------------------------------------------------------
%changelog
* Fri May 13 2016 (1.0.0)
  - Initial Cabal version
