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

#ifndef BACKENDS_FONTS_PROVIDER_H
#define BACKENDS_FONTS_PROVIDER_H

#include "graphics/font.h"

namespace Common {
class String;
}

/**
 * Bit flags for choosing the style of the font found
 */
enum FontStyle {
	/**
	 * The normal style
	 */
	kFontStyleNormal = 0,

	/**
	 * The bold version of a font
	 */
	kFontStyleBold = (1 << 0),

	/**
	 * The italic version of a font
	 */
	kFontStyleItalic = (1 << 1),

	/**
	 * Force the emulation of the style if a normal version of the font
	 * is found, but not the requested style.
	 */
	kFontStyleEmulate = (1 << 31)
};

/**
 * A class providing access to system fonts
 */
class SystemFontProvider {
public:
	virtual ~SystemFontProvider() {}

	/**
	 * Create a font for the given name and style. The font returned will be
	 * an exact match for the name requested, if present.
	 *
	 * @param name The name of the font face, not including style
	 * @param size The font size
	 * @param style A set of flags representing style
	 * @param render A set of flags for rendering the font
	 * @param dpi The DPI to load the font in
	 * @return A pointer to the font, or 0 if it could not be found
	 */
	virtual Graphics::Font *createFont(const Common::String &name, const Graphics::FontSize &size, uint32 style = kFontStyleNormal, Graphics::FontRenderMode render = Graphics::kFontRenderNormal, uint dpi = Graphics::kFontDefaultDPI) = 0;
};

#endif
