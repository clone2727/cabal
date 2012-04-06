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

#ifndef IMAGE_CODECS_SVQ3_H
#define IMAGE_CODECS_SVQ3_H

#include "graphics/pixelformat.h"
#include "graphics/surface.h"

#include "image/codecs/codec.h"

namespace Common {
class SeekableReadStream;
}

namespace Image {

class SVQ3Decoder : public Codec {
public:
	SVQ3Decoder(Common::SeekableReadStream *extraData);
	~SVQ3Decoder();

	const Graphics::Surface *decodeFrame(Common::SeekableReadStream &stream);
	Graphics::PixelFormat getPixelFormat() const { return _surface.format; }

private:
	Graphics::Surface _surface;

	bool _halfpel, _thirdpel;
	bool _lowDelay, _hasBFrames;
	bool _unknownFlag;
};

} // End of namespace Image

#endif
