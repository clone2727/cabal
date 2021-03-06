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

// Based on the ScummVM (GPLv2+) file of the same name

#ifdef WIN32

#include "backends/platform/sdl/win32/win32-window.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef ARRAYSIZE // winnt.h defines ARRAYSIZE, but we want our own one...

void SdlWindow_Win32::setupIcon() {
	HMODULE handle = GetModuleHandle(NULL);
	HICON   ico    = LoadIcon(handle, MAKEINTRESOURCE(1001 /* IDI_ICON */));
	if (ico) {
		SDL_SysWMinfo wminfo;
		if (getSDLWMInformation(&wminfo)) {
			// Replace the handle to the icon associated with the window class by our custom icon
#if SDL_VERSION_ATLEAST(2, 0, 0)
			SetClassLongPtr(wminfo.info.win.window, GCLP_HICON, (ULONG_PTR)ico);
#else
			SetClassLongPtr(wminfo.window, GCLP_HICON, (ULONG_PTR)ico);
#endif

			// Since there wasn't any default icon, we can't use the return value from SetClassLong
			// to check for errors (it would be 0 in both cases: error or no previous value for the
			// icon handle). Instead we check for the last-error code value.
			if (GetLastError() == ERROR_SUCCESS)
				return;
		}
	}

	// If no icon has been set, fallback to default path
	SdlWindow::setupIcon();
}

#endif
