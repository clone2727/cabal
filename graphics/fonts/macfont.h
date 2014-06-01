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

#ifndef GRAPHICS_FONTS_MACFONT_H
#define GRAPHICS_FONTS_MACFONT_H

#include "common/array.h"
#include "graphics/font.h"

namespace Common {
class SeekableReadStream;
}

namespace Graphics {

// TODO: Mac font family (FOND) support

/**
 * A Mac FONT or NFNT resource font.
 */
class MacFont : public Font {
public:
	MacFont();
	~MacFont();

	/** Load a font. */
	bool loadFont(Common::SeekableReadStream &stream);

	/** Close this font */
	void close();

	// Font API
	int getFontHeight() const { return _rectHeight; }
	int getMaxCharWidth() const { return _maxWidth; }
	int getCharWidth(uint32 chr) const;
	void drawChar(Surface *dst, uint32 chr, int x, int y, uint32 color) const;
	int getKerningOffset(uint32 left, uint32 right) const;

private:
	uint16 _rectHeight;
	uint16 _maxWidth;
	uint32 _firstChar;
	uint32 _lastChar;

	byte *_bitImage;
	uint16 _bitImageRowWidth;

	struct Glyph {
		void clear() {
			bitmapOffset = 0;
			width = 0;
			bitmapWidth = 0;
			kerningOffset = 0;
		}

		uint16 bitmapOffset;
		byte width;
		uint16 bitmapWidth;
		int kerningOffset;
	};

	Common::Array<Glyph> _glyphs;
	Glyph _defaultChar;
	const Glyph *findGlyph(uint32 c) const;
};

} // End of namespace Graphics

#endif
