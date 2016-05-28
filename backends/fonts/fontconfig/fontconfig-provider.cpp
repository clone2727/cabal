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

#include <fontconfig/fontconfig.h>

#include "backends/fonts/ttf/ttf-provider.h" 
#include "common/debug.h"
#include "common/fs.h"
#include "graphics/fonts/font-properties.h"

class FontconfigFontProvider : public TTFFontProvider {
public:
	FontconfigFontProvider();
	~FontconfigFontProvider();

protected:
	Common::SeekableReadStream *createReadStreamForFont(const Common::String &name, uint32 style);

private:
	FcConfig *_config;
};

FontconfigFontProvider::FontconfigFontProvider() : _config(0) {
	if (!FcInit()) {
		warning("Failed to initialize fontconfig");
		return;
	}

	_config = FcInitLoadConfigAndFonts();
	if (!_config) {
		warning("Failed to create fontconfig config");
		FcFini();
	}
}

FontconfigFontProvider::~FontconfigFontProvider() {
	if (_config) {
		FcConfigDestroy(_config);
		FcFini();
	}
}

Common::SeekableReadStream *FontconfigFontProvider::createReadStreamForFont(const Common::String &name, uint32 style) {
	if (!_config)
		return 0;

	Common::String styleName = Graphics::getFontStyleString(style);

	FcPattern *pattern = FcPatternCreate();

	// Match on the family name
	FcValue value;
	value.type = FcTypeString;
	value.u.s = reinterpret_cast<const FcChar8 *>(name.c_str());
	if (!FcPatternAdd(pattern, FC_FAMILY, value, FcFalse)) {
		FcPatternDestroy(pattern);
		return 0;
	}

	// Match on the style string
	value.u.s = reinterpret_cast<const FcChar8 *>(styleName.c_str());
	if (!FcPatternAdd(pattern, FC_STYLE, value, FcFalse)) {
		FcPatternDestroy(pattern);
		return 0;
	}

	// Only allow TrueType fonts
	// Might change in the future.
	value.u.s = reinterpret_cast<const FcChar8 *>("TrueType");
	if (!FcPatternAdd(pattern, FC_FONTFORMAT, value, FcFalse)) {
		FcPatternDestroy(pattern);
		return 0;
	}

	// Call these functions
	// Might be doing pattern matching. Documentation is awful.
	FcConfigSubstitute(0, pattern, FcMatchPattern);
	FcDefaultSubstitute(pattern);

	// Fetch the match
	FcResult result;
	FcFontSet* fontSet = FcFontSort(0, pattern, 0, 0, &result);
	FcPatternDestroy(pattern);

	if (!fontSet)
		return 0;

	for (int i = 0; i < fontSet->nfont; i++) {
		FcPattern *foundPattern = fontSet->fonts[i];

		// Get the family name of the font
		FcChar8 *familyName;
		if (FcPatternGetString(foundPattern, FC_FAMILY, 0, &familyName) != FcResultMatch)
			continue;

		// If we don't actually match, bail out. We don't want to end
		// up with the default fontconfig font, which would look horrible.
		if (!name.equalsIgnoreCase(reinterpret_cast<const char *>(familyName)))
			continue;

		// Get the name of the font
		FcChar8 *fileName;
		if (FcPatternGetString(foundPattern, FC_FILE, 0, &fileName) != FcResultMatch)
			continue;

		// Let's make sure we can actually get that
		Common::FSNode fontNode(reinterpret_cast<const char *>(fileName));
		if (!fontNode.exists() || !fontNode.isReadable())
			continue;

		debug(1, "Matched %s %s -> '%s'", name.c_str(), styleName.c_str(), fontNode.getPath().c_str());

		FcFontSetDestroy(fontSet);
		return fontNode.createReadStream();
	}

	// We ain't found s#&t
	FcFontSetDestroy(fontSet);
	return 0;
}

SystemFontProvider *createFontconfigFontProvider() {
	return new FontconfigFontProvider();
}
