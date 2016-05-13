/* Cabal - Legacy Game Implementations
 *
 * Cabal is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef CABAL_BASE_VERSION_H
#define CABAL_BASE_VERSION_H

namespace Cabal {

/**
 * Get the Cabal version
 *
 * e.g. "1.0.0"
 */
const char *getVersion();

/**
 * Get the build date of the library
 * e.g. "2016-05-13"
 */
const char *getBuildDate();

/**
 * Get a string containing the version and the build date
 * e.g. "1.0.0 (2016-05-13)
 */
const char *getVersionDate();

/**
 * Get a string containing the name, version, and build date
 * e.g. "Cabal 1.0.0 (2016-05-13)
 */
const char *getFullVersion();

/**
 * Get a string containing features supported by this build
 * e.g. "ALSA PNG zLib"
 */
const char *getFeatures();

} // End of namespace Cabal

#endif
