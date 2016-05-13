#ifndef INCLUDED_FROM_BASE_VERSION_CPP
#error This file may only be included by base/version.cpp
#endif

// Reads revision number from file
// (this is used when building with Visual Studio)
#ifdef CABAL_INTERNAL_REVISION
#include "internal_revision.h"
#endif

#ifdef RELEASE_BUILD
#undef CABAL_REVISION
#endif

#ifndef CABAL_REVISION
#define CABAL_REVISION
#endif

#define CABAL_VERSION "1.0.0git" CABAL_REVISION
