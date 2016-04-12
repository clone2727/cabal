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

#include "backends/fonts/ttf/ttf-provider.h"
#include "common/debug.h"
#include "common/ptr.h"
#include "graphics/fonts/ttf.h"

Graphics::Font *TTFFontProvider::createFont(const Common::String &name, const Graphics::FontSize &size, uint32 style, Graphics::FontRenderMode render, uint dpi) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(createReadStreamForFont(name, style));
	bool emulateMode = false;
	if (!stream && (style & kFontStyleEmulate) != 0) {
		stream.reset(createReadStreamForFont(name, kFontStyleNormal));
		emulateMode = true;
	}

	if (!stream)
		return 0;

	// TODO: Emulate if necessary

	return Graphics::loadTTFFont(*stream, size, dpi, render);
}

Common::String TTFFontProvider::makeStyleString(uint32 style) {
	// Ignore force
	style &= ~kFontStyleEmulate;

	switch (style) {
	case kFontStyleNormal:
		return "Regular";
	case kFontStyleBold:
		return "Bold";
	case kFontStyleItalic:
		return "Italic";
	case kFontStyleBold | kFontStyleItalic:
		return "Bold Italic";
	}

	// Default to regular
	return "Regular";
}
