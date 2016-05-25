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

#ifndef GRAPHICS_FONT_PROPERTIES_H
#define GRAPHICS_FONT_PROPERTIES_H

#include "common/hash-str.h"
#include "common/scummsys.h"

namespace Common {
class Archive;
class FSNode;
}

namespace Graphics {

/**
 * A representation of a font's properties for identification purposes
 */
class FontProperties {
public:
	FontProperties(const Common::String &family, const Common::String &style) :
		_family(family), _style(style) {}

	/**
	 * Get the family name of the font
	 */
	const Common::String &getFamily() const { return _family; }

	/**
	 * Get the style of the font
	 */
	const Common::String &getStyle() const { return _style; }

private:
	/**
	 * The family of the font
	 */
	Common::String _family;

	/**
	 * The style of the font
	 */
	Common::String _style;
};

/**
 * Equality operator for FontProperties
 */
inline bool operator==(const FontProperties &x, const FontProperties &y) {
	return x.getFamily().equalsIgnoreCase(y.getFamily()) && x.getStyle().equalsIgnoreCase(y.getStyle());
}

/**
 * Hash function for FontProperties
 */
struct FontPropertiesHash {
	uint operator()(const FontProperties &x) const {
		return Common::hashit_lower(x.getFamily()) ^ Common::hashit_lower(x.getStyle());
	}
};

/**
 * Hash map mapping from FontProperties to a path containing the font, suitable
 * for use with Common::FSNode.
 */
typedef Common::HashMap<FontProperties, Common::String, FontPropertiesHash> FontPropertyMap;

#ifdef USE_FREETYPE2

/**
 * Scan for TTF fonts in a directory
 */
FontPropertyMap scanDirectoryForTTF(const Common::String &path);

/**
 * Scan for TTF fonts in a directory
 */
FontPropertyMap scanDirectoryForTTF(const Common::FSNode &node);

/**
 * Scan for TTF fonts in an archive
 */
FontPropertyMap scanArchiveForTTF(const Common::Archive &archive);

#endif

} // End of namespace Graphics

#endif
