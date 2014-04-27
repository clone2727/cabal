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

#include "common/stream.h"
#include "common/system.h"
#include "common/textconsole.h"	
#include "graphics/surface.h"

#include "mystjag/graphics.h"

namespace MystJaguar {

GraphicsManager::GraphicsManager() {
}

GraphicsManager::~GraphicsManager() {
}

template <typename PixelInt>
void readSurface(Graphics::Surface &surface, Common::SeekableReadStream &stream) {
	// Weirdest. color. format. ever. RBG556
	Graphics::PixelFormat srcFormat(2, 5, 6, 5, 0, 11, 0, 6, 0);
	PixelInt *dst = (PixelInt *)surface.getPixels();

	for (int32 i = 0; i < surface.w * surface.h; i++) {
		uint16 color = stream.readUint16BE();
		byte r, g, b;
		srcFormat.colorToRGB(color, r, g, b);

		*dst++ = surface.format.RGBToColor(r, g, b);
	}
}

Graphics::Surface *GraphicsManager::decodeImage(Common::SeekableReadStream &stream) {
	uint16 width = stream.readUint16BE();
	uint16 height = stream.readUint16BE();

	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(width, height, g_system->getScreenFormat());

	if (surface->format.bytesPerPixel == 2)
		readSurface<uint16>(*surface, stream);
	else
		readSurface<uint32>(*surface, stream);

	return surface;
}

} // End of namespace MystJaguar
