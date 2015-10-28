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

#ifndef GRAPHICS_FONTS_TTF_H
#define GRAPHICS_FONTS_TTF_H

#include "common/scummsys.h"

#ifdef USE_FREETYPE2

#include "common/stream.h"
#include "graphics/font.h"

namespace Common {
class FSNode;
}

namespace Graphics {

/**
 * This specifies how the font size is defined.
 */
enum TTFSizeMode {
	/**
	 * Character height only.
	 *
	 * This matches rendering obtained when calling
	 * CreateFont in Windows with negative height values.
	 */
	kTTFSizeModeCharacter,

	/**
	 * Full cell height.
	 *
	 * This matches rendering obtained when calling
	 * CreateFont in Windows with positive height values.
	 */
	kTTFSizeModeCell
};

/**
 * Loads a TTF font file from a given data stream object.
 *
 * @param stream     Stream object to load font data from.
 * @param size       The point size to load.
 * @param sizeMode   The point size definition used for the size parameter.
 * @param dpi        The dpi to use for size calculations, by default 72dpi
 *                   are used.
 * @param renderMode FreeType2 mode used to render glyphs. @see FontRenderMode
 * @param mapping    A mapping from code points 0-255 into UTF-32 code points.
 *                   This can be used to support various 8bit character sets.
 *                   In case the msb of the UTF-32 code point is set the font
 *                   loading fails in case no glyph for it is found. When this
 *                   is non-null only characters given in the mapping are
 *                   supported.
 * @return 0 in case loading fails, otherwise a pointer to the Font object.
 */
Font *loadTTFFont(Common::SeekableReadStream &stream, int size, TTFSizeMode sizeMode = kTTFSizeModeCharacter, uint dpi = kFontDefaultDPI, FontRenderMode renderMode = kFontRenderLight, const uint32 *mapping = 0);

/**
 * Get the font family and style for a TTF font
 * @param path The path to the font
 * @param family The family name
 * @param style The style name
 * @return true on success, false otherwise
 */
bool getTTFDetails(const Common::String &path, Common::String &family, Common::String &style);

void shutdownTTF();

} // End of namespace Graphics

#endif

#endif

