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

#include <sys/types.h>
#include <pwd.h>

#include "backends/fonts/macosx/macosx-font-provider.h"
#include "backends/fonts/ttf/ttf-provider.h"
#include "common/debug.h"
#include "common/fs.h"
#include "graphics/fonts/font-properties.h"

class MacOSXFontProvider : public TTFFontProvider {
protected:
	Common::SeekableReadStream *createReadStreamForFont(const Common::String &name, uint32 style);

private:
	static Common::SeekableReadStream *findFontInDirectory(const Common::String &path, const Common::String &name, const Common::String &style);
};

Common::SeekableReadStream *MacOSXFontProvider::createReadStreamForFont(const Common::String &name, uint32 style) {
	Common::String styleName = Graphics::getFontStyleString(style);

	// Try the user's font directory
	// Use the BSD interface so we don't have to touch Obj-C (which is always a plus)
	const passwd *passwdEntry = getpwuid(getuid());
	if (passwdEntry) {
		Common::FSNode userFontNode(Common::FSNode(passwdEntry->pw_dir).getChild("Library").getChild("Fonts"));
		Common::SeekableReadStream *stream = findFontInDirectory(userFontNode.getPath(), name, styleName);
		if (stream)
			return stream;
	}

	// Try the library path
	Common::SeekableReadStream *stream = findFontInDirectory("/Library/Fonts", name, styleName);
	if (stream)
		return stream;

	// Last resort: system font directory
	stream = findFontInDirectory("/System/Library/Fonts", name, styleName);
	if (stream)
		return stream;

	// Failed to find the font
	return 0;
}

Common::SeekableReadStream *MacOSXFontProvider::findFontInDirectory(const Common::String &path, const Common::String &name, const Common::String &style) {
	Common::FSNode node(path);
	if (!node.exists() || !node.isDirectory())
		return 0;

	// Look for normal TTF fonts first
	Graphics::FontPropertyMap map = Graphics::scanDirectoryForTTF(node);
	Graphics::FontPropertyMap::const_iterator mapIter = map.find(Graphics::FontProperties(name, style));
	if (mapIter != map.end()) {
		Common::FSNode fileNode(mapIter->_value);
		if (fileNode.isReadable())
			return fileNode.createReadStream();
	}

	// TODO: Scan for resource fork based TTF files
	return 0;
}

SystemFontProvider *createMacOSXFontProvider() {
	return new MacOSXFontProvider();
}
