#------------------------------------------------------------------------------
#   cabal-tools.spec
#       This SPEC file controls the building of Cabal Tools RPM packages.
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#   Prologue information
#------------------------------------------------------------------------------
Name		: cabal-tools
Version		: 1.0.0git
Release		: 1
Summary		: Cabal-related tools
Group		: Interpreters
License		: GPL

Url             : http://project-cabal.org

Source		: %{name}-%{version}.tar.bz2
Source1		: libmad-0.15.1b.tar.bz2
BuildRoot	: %{_tmppath}/%{name}-%{version}-root

BuildRequires	: zlib-devel
BuildRequires	: wxGTK-devel
#------------------------------------------------------------------------------
#   Description
#------------------------------------------------------------------------------
%description
Tools for compressing data files for Cabal-related games and other tools.

#------------------------------------------------------------------------------
#   install scripts
#------------------------------------------------------------------------------
%prep
%setup -q -a 1 -n cabal-tools-%{version}

%build
(cd libmad-0.15.1b; grep -v 'force-\(mem\|addr\)' configure > configure.new; mv -f configure.new configure; chmod 700 configure; ./configure --enable-static --disable-shared --prefix=%{_builddir}/cabalexec-%{version}/tmp; make; make install)
./configure --with-mad-prefix=%{_builddir}/cabalexec-%{version}/tmp
make

%install
install -m755 -d %{buildroot}%{_bindir}
install -m755 -D create_sjisfnt %{buildroot}%{_bindir}
install -m755 -D cabal-tools{,-cli} %{buildroot}%{_bindir}
install -m755 -D de{cine,gob,kyra,riven,scumm,sword2} %{buildroot}%{_bindir}
install -m755 -D {construct,extract}_mohawk %{buildroot}%{_bindir}

%clean
rm -Rf ${RPM_BUILD_ROOT}

#------------------------------------------------------------------------------
#   Files listing.
#------------------------------------------------------------------------------
%files
%doc README COPYING
%attr(0755,root,root)%{_bindir}/cabalexec*
%attr(0755,root,root)%{_bindir}/create_sjisfnt
%attr(0755,root,root)%{_bindir}/de*
%attr(0755,root,root)%{_bindir}/extract_*
%attr(0755,root,root)%{_bindir}/construct_*

#------------------------------------------------------------------------------
#   Change Log
#------------------------------------------------------------------------------
%changelog
* Fri May 13 2016 (1.0.0)
  - Initial Cabal version
