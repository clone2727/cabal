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

#ifndef BACKENDS_FONTS_TTF_PROVIDER_H
#define BACKENDS_FONTS_TTF_PROVIDER_H

#include "common/scummsys.h"

#ifdef USE_FREETYPE2

#include "backends/fonts/font-provider.h"

namespace Common {
class SeekableReadStream;
}

/**
 * The default font provider which handles creating TTF fonts
 * automatically using the TTF class.
 */
class TTFFontProvider : public SystemFontProvider {
public:
	// SystemFontProvider API
	Graphics::Font *createFont(const Common::String &name, const Graphics::FontSize &size, uint32 style = Graphics::kFontStyleNormal, Graphics::FontRenderMode render = Graphics::kFontRenderNormal, uint dpi = Graphics::kFontDefaultDPI);

protected:
	/**
	 * Create a SeekableReadStream for the given font
	 *
	 * @param name The name of the font face, not including style
	 * @param style A set of flags representing style
	 * @return The pointer to the TTF stream, or 0 if it could not be found
	 */
	virtual Common::SeekableReadStream *createReadStreamForFont(const Common::String &name, uint32 style = Graphics::kFontStyleNormal) = 0;
};

#endif

#endif


