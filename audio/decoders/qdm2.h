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

// Based on the ScummVM (GPLv2+) file of the same name

#ifndef AUDIO_QDM2_H
#define AUDIO_QDM2_H

#include "common/types.h"

namespace Common {
class SeekableReadStream;
}

namespace Audio {

class Codec;
class PacketizedAudioStream;

/**
 * Create a new Codec from the QDM2 data in the given stream.
 *
 * @param extraData           the QuickTime extra data stream
 * @return   a new Codec, or NULL, if an error occurred
 */
Codec *makeQDM2Decoder(Common::SeekableReadStream &extraData);

/**
 * Create a PacketizedAudioStream that decodes QDM2 sound
 *
 * @param extraData  The stream containing the extra data needed for initialization
 * @return             A new PacketizedAudioStream, or NULL on error
 */
PacketizedAudioStream *makeQDM2Stream(Common::SeekableReadStream &extraData);

} // End of namespace Audio

#endif
