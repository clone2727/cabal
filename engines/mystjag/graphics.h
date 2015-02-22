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

#ifndef MYSTJAG_GRAPHICS_H
#define MYSTJAG_GRAPHICS_H

#include "common/scummsys.h"

#include "mystjag/session.h"

namespace Common {
class SeekableReadStream;
}

namespace Graphics {
struct Surface;
}

namespace MystJaguar {

enum Cursor {
	kCursorNone = -1,
	kCursorUp = 0,
	kCursorDown,
	kCursorLeft,
	kCursorRight,
	kCursorWhitePage,
	kCursorBluePage,
	kCursorRedPage,
	kCursorFlat,
	kCursorGrab,
	kCursorKey,
	kCursorMatch,
	kCursorStrikingMatch,
	kCursorLitMatch,
	kCursorRotateLeft,
	kCursorRotateRight,
	kCursorZip,
	kCursorArrow,
	kCursorZoomIn,
	kCursorZoomOut,
	kCursorTurnLeft,
	kCursorTurnRight,
	kCursorTurnBoth
};

class GraphicsManager {
public:
	GraphicsManager(SessionManager *session);
	~GraphicsManager();

	Graphics::Surface *decodeImage(Common::SeekableReadStream &stream);
	void setCursor(Cursor cursor);

private:
	SessionManager *_session;
	Cursor _lastCursor;

	/**
	 * Load a hardcoded image out of the game's executable
	 */
	Graphics::Surface *loadExecutableImage(BinDataType dataType, uint16 width, uint16 height);
};

} // End of namespace MystJaguar

#endif
