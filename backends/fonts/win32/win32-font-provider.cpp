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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "backends/fonts/ttf/ttf-provider.h" 
#include "backends/fonts/win32/win32-font-provider.h"
#include "common/debug.h"
#include "common/fs.h"
#include "graphics/fonts/font-properties.h"

class Win32FontProvider : public TTFFontProvider {
protected:
	Common::SeekableReadStream *createReadStreamForFont(const Common::String &name, uint32 style);
};

Common::SeekableReadStream *Win32FontProvider::createReadStreamForFont(const Common::String &name, uint32 style) {
	// Get the Windows directory path
	char buffer[MAX_PATH];
	int result = GetWindowsDirectoryA(buffer, sizeof(buffer));
	if (result <= 0 || result > (int)sizeof(buffer)) {
		warning("Failed to get Windows path");
		return 0;
	}

	// Search its "Fonts" child for fonts
	Common::FSNode fontNode(Common::FSNode(buffer).getChild("Fonts"));
	if (!fontNode.exists() || !fontNode.isDirectory()) {
		warning("Failed to find fonts directory");
		return 0;
	}

	Graphics::FontProperties property(name, makeStyleString(style));
	Graphics::FontPropertyMap fontMap = Graphics::scanDirectoryForTTF(fontNode.getPath());
	Graphics::FontPropertyMap::iterator it = fontMap.find(property);
	if (it != fontMap.end()) {
		debug("Matched on %s", it->_value.c_str());
		Common::FSNode child(it->_value);
		return child.createReadStream();
	}

	// Failed to find the font
	return 0;
}

SystemFontProvider *createWin32FontProvider() {
	return new Win32FontProvider();
}
