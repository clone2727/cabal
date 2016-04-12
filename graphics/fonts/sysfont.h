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

#ifndef GRAPHICS_FONTS_SYSFONT_H
#define GRAPHICS_FONTS_SYSFONT_H

#include "backends/fonts/font-provider.h"
#include "common/singleton.h"

namespace Graphics {

class Font;
class FontSize;

class SystemFontManager : public Common::Singleton<SystemFontManager> {
public:
	/**
	 * Request a font from a system. This can be one installed by the user, one installed
	 * by the operating system, or one overridden and provided with Cabal.
	 *
	 * @param name
	 *     The name of the font family. For example "arial" for pulling an Arial font.
	 * @param size
	 *     The size of the font.
	 * @param style
	 *     The style of the font.
	 * @param render
	 *     The method to render the font.
	 * @param dpi
	 *     The dots per inch to render the font with.
	 */
	Graphics::Font *createFont(const Common::String &name, const FontSize &size, uint32 style = kFontStyleNormal, FontRenderMode render = kFontRenderNormal, uint dpi = kFontDefaultDPI);

	/**
	 * Find a matching font properties for a Windows font file name.
	 * This is used for engines like Wintermute which have this file name
	 * in game data instead of the family name.
	 *
	 * @param fileName The name of the Windows font file
	 * @param name The family name, returned
	 * @param style The style, returned
	 * @return true on finding a match, false otherwise
	 */
	bool matchWindowsFontName(const Common::String &fileName, Common::String &name, uint32 &style) const;

private:
	friend class Common::Singleton<SingletonBaseType>;
	SystemFontManager() {}
	~SystemFontManager() {}

	/**
	 * Internal function for creating a font; called by createFont()
	 */
	Graphics::Font *createFontIntern(const Common::String &name, const FontSize &size, uint32 style = kFontStyleNormal, FontRenderMode render = kFontRenderNormal, uint dpi = kFontDefaultDPI);
};

} // End of namespace Graphics

#define SystemFontMan (::Graphics::SystemFontManager::instance())

#endif
