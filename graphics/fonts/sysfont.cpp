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

#include "common/config-manager.h"
#include "common/system.h"
#include "graphics/fonts/sysfont.h"

namespace Common {
DECLARE_SINGLETON(Graphics::SystemFontManager);
}

namespace Graphics {

Graphics::Font *SystemFontManager::createFont(const Common::String &name, uint size, uint32 style, FontRenderMode render, uint dpi) {
	SystemFontProvider *provider = g_system->getSystemFontProvider();

	if (provider) {
		Graphics::Font *font = provider->createFont(name, size, style, render, dpi);

		if (font)
			return font;
	}

	return 0;
}

} // End of namespace Graphics
