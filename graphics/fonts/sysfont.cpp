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

#include "common/archive.h"
#include "common/config-manager.h"
#include "common/stream.h"
#include "common/system.h"
#include "graphics/fonts/font-properties.h"
#include "graphics/fonts/sysfont.h"
#ifdef USE_FREETYPE2
#include "graphics/fonts/ttf.h"
#endif

namespace Common {
DECLARE_SINGLETON(Graphics::SystemFontManager);
}

namespace Graphics {

struct SubstituteFont {
	const char *familyName;
	uint32 style;
	const char *subFamilyName;
	uint32 subStyle;
};

static const SubstituteFont s_substituteFonts[] = {
	{ "Arial", kFontStyleNormal, "FreeSans", kFontStyleNormal },
	{ "Arial", kFontStyleBold, "FreeSans", kFontStyleBold },
	{ "MS Gothic", kFontStyleNormal, "Ume Gothic", kFontStyleNormal }
};

Graphics::Font *SystemFontManager::createFont(const Common::String &name, const FontSize &size, uint32 style, FontRenderMode render, uint dpi) {
	// Look for the font directly
	Graphics::Font *font = createFontIntern(name, size, style, render, dpi);
	if (font)
		return font;

	// Look for substitute fonts
	for (int i = 0; i < ARRAYSIZE(s_substituteFonts); i++) {
		if (!name.equalsIgnoreCase(s_substituteFonts[i].familyName))
			continue;

		// Compare without emulation
		if (s_substituteFonts[i].style != (style & ~kFontStyleEmulate))
			continue;

		// Found a match without emulation; attempt to find the font
		font = createFontIntern(s_substituteFonts[i].subFamilyName, size, s_substituteFonts[i].subStyle, render, dpi);
		if (font)
			return font;
	}

	// Look for substitute fonts forcing emulation
	if ((style & kFontStyleEmulate) != 0) {
		for (int i = 0; i < ARRAYSIZE(s_substituteFonts); i++) {
			if (!name.equalsIgnoreCase(s_substituteFonts[i].familyName))
				continue;

			// Look for normal ones, so we can use emulation mode
			if (s_substituteFonts[i].style != kFontStyleNormal)
				continue;

			// Found a match without emulation; attempt to find the font
			font = createFontIntern(s_substituteFonts[i].subFamilyName, size, style, render, dpi);
			if (font)
				return font;
		}
	}

	return 0;
}

Graphics::Font *SystemFontManager::createFontIntern(const Common::String &name, const FontSize &size, uint32 style, FontRenderMode render, uint dpi) {
#ifdef USE_FREETYPE2
	// Attempt to find the font in the game's search set
	{
		FontPropertyMap gameFonts = scanArchiveForTTF(SearchMan);
		FontPropertyMap::const_iterator it = gameFonts.find(FontProperties(name, Graphics::getFontStyleString(style)));
		if (it != gameFonts.end()) {
			Common::ScopedPtr<Common::SeekableReadStream> stream(SearchMan.createReadStreamForMember(it->_value));
			if (stream) {
				Font *font = loadTTFFont(*stream, size, dpi, render);
				if (font)
					return font;
			}
		}
	}
#endif

	// Actually query the system for the font
	SystemFontProvider *provider = g_system->getSystemFontProvider();
	if (provider) {
		Graphics::Font *font = provider->createFont(name, size, style, render, dpi);

		if (font)
			return font;
	}

	// TODO: Last resort: check the theme

	return 0;
}

// A struct to hold a Windows font name mapping
struct WindowsFontNameMap {
	const char *fileName;
	const char *family;
	uint32 style;
};

// TODO: Add more to the table (as needed, only)
static const WindowsFontNameMap s_windowsFontNameMap[] = {
	{ "arial.ttf", "Arial", kFontStyleNormal }
};

bool SystemFontManager::matchWindowsFontName(const Common::String &fileName, Common::String &name, uint32 &style) const {
	for (uint32 i = 0; i < ARRAYSIZE(s_windowsFontNameMap); i++) {
		if (fileName.equalsIgnoreCase(s_windowsFontNameMap[i].fileName)) {
			name = s_windowsFontNameMap[i].family;
			style = s_windowsFontNameMap[i].style;
			return true;
		}
	}

	return false;
}

} // End of namespace Graphics
