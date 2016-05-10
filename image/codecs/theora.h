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

#ifndef IMAGE_CODECS_THEORA_H
#define IMAGE_CODECS_THEORA_H

#include "common/scummsys.h"

#ifdef USE_THEORADEC

namespace Image {

class Codec;

/**
 * Create a codec capable of decoding Theora frames
 *
 * @param extraData
 *    The vorbis header data, combined into one stream, as used in non-ogg
 *    containers.
 * @return a new Codec, or NULL if an error occurred
 */
Codec *makeTheoraDecoder(Common::SeekableReadStream &extraData);

/**
 * Create a codec capable of decoding Theora frames
 *
 * @param packet1
 *    The first theora header, as used in ogg
 * @param packet2
 *    The second theora header, as used in ogg
 * @param packet3
 *    The third theora header, as used in ogg
 * @return a new Codec, or NULL if an error occurred
 */
Codec *makeTheoraDecoder(
	Common::SeekableReadStream &packet1,
	Common::SeekableReadStream &packet2,
	Common::SeekableReadStream &packet3);

} // End of namespace Image

#endif

#endif
