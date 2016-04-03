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

#ifndef UNITY_GRAPHICS_H
#define UNITY_GRAPHICS_H

#include "unity.h"
#include "sprite_player.h"

#include "common/rect.h"

namespace Graphics {
	class Font;
}

namespace Unity {

class SpritePlayer;

struct Image {
	unsigned int width;
	unsigned int height;
	byte *data;
};

struct MRGFile {
	Common::Array<uint16> widths, heights;
	Common::Array<byte *> data;

	~MRGFile() {
		for (unsigned int i = 0; i < data.size(); i++)
			delete[] data[i];
	}
};

class Graphics {
public:
	Graphics(UnityEngine *engine);
	~Graphics();

	void init();

	void setCursor(unsigned int id, bool wait);
	void setBackgroundImage(Common::String filename);

	void loadMRG(Common::String filename, MRGFile *mrg);

	::Graphics::Font *getFont(unsigned int id) const;
	void drawString(uint x, uint y, const Common::String &text, uint font) const;
	uint getFontHeight(uint font) const;
	uint getStringWidth(const Common::String &text, uint font) const;

	void drawMRG(MRGFile *mrg, unsigned int entry, unsigned int x, unsigned int y);
	void drawBackgroundImage();
	void drawSprite(SpritePlayer *sprite, int x, int y, unsigned int scale = 256);
	void drawBackgroundPolys(Common::Array<struct ScreenPolygon> &polys);
	void renderPolygonEdge(Common::Array<Common::Point> &points, byte colour);

	void fillRect(byte colour, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);

	void playMovie(Common::String filename);

protected:
	void loadPalette();
	void loadCursors();
	void loadFonts();

	void blit(byte *data, int x, int y, unsigned int width, unsigned int height, byte transparent = COLOUR_BLANK);

	UnityEngine *_vm;
	byte *_basePalette, *_palette;
	Image _background;
	Common::Array<Image> _cursors;
	Common::Array<Image> _waitCursors;

	Common::Array< ::Graphics::Font *> _fonts;
};

}

#endif

