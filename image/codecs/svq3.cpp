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

// Code here based off of ffmpeg

#include "image/codecs/svq3.h"

#include "common/debug.h"
#include "common/bitstream.h"
#include "common/stream.h"
#include "common/system.h"

namespace Image {

SVQ3Decoder::SVQ3Decoder(Common::SeekableReadStream *extraData) : Codec() {
	assert(extraData);
	extraData->seek(0);

	if (extraData->readUint32BE() != MKTAG('S', 'E', 'Q', 'H'))
		error("Invalid SVQ3 extra data -- missing SEQH");

	/* uint32 bitStreamSize = */ extraData->readUint32BE();

	Common::BitStream8MSB bits(*extraData);

	uint16 width, height;
	switch (bits.getBits(3)) {
	case 0:
		width = 160;
		height = 120;
		break;
	case 1:
		width = 128;
		height = 96;
		break;
	case 2:
		width = 176;
		height = 144;
		break;
	case 3:
		width = 352;
		height = 288;
		break;
	case 4:
		width = 704;
		height = 576;
		break;
	case 5:
		width = 240;
		height = 180;
		break;
	case 6:
		width = 320;
		height = 240;
		break;
	default:
		width = bits.getBits(12);
		height = bits.getBits(12);
		break;
	}

	_surface.create(width, height, g_system->getScreenFormat());

	_halfpel = bits.getBit() != 0;
	_thirdpel = bits.getBit() != 0;

	bits.skip(4); // unknown

	_lowDelay = bits.getBit() != 0;

	bits.getBit(); // unknown

	while (bits.getBit())
		bits.skip(8);

	_unknownFlag = bits.getBit() != 0;
	_hasBFrames = !_lowDelay;

	if (_unknownFlag)
		error("Unhandled SVQ3 watermark");
}

SVQ3Decoder::~SVQ3Decoder() {
	_surface.free();
}

const Graphics::Surface *SVQ3Decoder::decodeFrame(Common::SeekableReadStream &stream) {
	// TODO
	return &_surface;
}

} // End of namespace Image
