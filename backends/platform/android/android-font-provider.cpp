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

#include "backends/platform/android/android-font-provider.h" 
#include "backends/fonts/ttf/ttf-provider.h"
#include "common/debug.h"
#include "common/fs.h"
#include "graphics/fonts/font-properties.h"

class AndroidFontProvider : public TTFFontProvider {
protected:
	Common::SeekableReadStream *createReadStreamForFont(const Common::String &name, uint32 style);
};

Common::SeekableReadStream *AndroidFontProvider::createReadStreamForFont(const Common::String &name, uint32 style) {
	// Android stores its system fonts, appropriately in /system/fonts
	Common::FSNode fontNode("/system/fonts");
	if (!fontNode.exists() || !fontNode.isDirectory()) {
		warning("Failed to find fonts directory");
		return 0;
	}

	Graphics::FontProperties property(name, Graphics::getFontStyleString(style));
	Graphics::FontPropertyMap fontMap = Graphics::scanDirectoryForTTF(fontNode.getPath());
	Graphics::FontPropertyMap::iterator it = fontMap.find(property);
	if (it != fontMap.end()) {
		Common::FSNode child(it->_value);
		return child.createReadStream();
	}

	// Failed to find the font
	return 0;
}

SystemFontProvider *createAndroidFontProvider() {
	return new AndroidFontProvider();
}

