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

#include "common/stream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/cursorman.h"
#include "graphics/surface.h"

#include "mystjag/graphics.h"

namespace MystJaguar {

GraphicsManager::GraphicsManager(SessionManager *session) : _session(session) {
	_lastCursor = kCursorNone;
}

GraphicsManager::~GraphicsManager() {
}

namespace {

Graphics::Surface *readImage(Common::SeekableReadStream &stream, uint16 width, uint16 height) {
	// Weirdest. color. format. ever. RBG556
	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(width, height, Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 0, 6, 0));

	uint16 *dst = (uint16 *)surface->getPixels();
	for (int32 i = 0; i < width * height; i++)
		*dst++ = stream.readUint16BE();

	return surface;
}

} // End of anonymous namespace

Graphics::Surface *GraphicsManager::decodeImage(Common::SeekableReadStream &stream) {
	uint16 width = stream.readUint16BE();
	uint16 height = stream.readUint16BE();

	return readImage(stream, width, height);
}

Graphics::Surface *GraphicsManager::loadExecutableImage(BinDataType dataType, uint16 width, uint16 height) {
	Common::SeekableReadStream *stream = _session->loadExecutableData(dataType, width * height * 2);
	Graphics::Surface *surface = readImage(*stream, width, height);
	delete stream;
	return surface;
}

void GraphicsManager::setCursor(Cursor cursor) {
	// Bail out if the cursor hasn't changed
	if (_lastCursor == cursor)
		return;

	_lastCursor = cursor;

	if (cursor == kCursorNone) {
		// No cursor -> hide it
		CursorMan.showMouse(false);
		return;
	}

	// Otherwise, show the mouse
	CursorMan.showMouse(true);

	static const int hotspots[][2] = {
		{ 6, 1 },
		{ 6, 15 },
		{ 3, 6 },
		{ 12, 6 },
		{ 4, 7 },
		{ 4, 7 },
		{ 4, 7 },
		{ 8, 6 },
		{ 8, 6 },
		{ 2, 8 },
		{ 2, 5 },
		{ 2, 5 },
		{ 2, 5 },
		{ 8, 8 },
		{ 8, 8 },
		{ 8, 8 },
		{ 16, 1 },
		{ 4, 3 },
		{ 4, 3 },
		{ 8, 7 },
		{ 8, 7 },
		{ 8, 7 }
	};

	// Get the image out of the executable
	Graphics::Surface *surface = loadExecutableImage(kBinDataCursors, 384, 17);

	// Due to the stupidity of the cursor code not taking a pitch, we need to
	// actually generate a subimage...
	Graphics::Surface temp;
	temp.create(16, 17, surface->format);

	for (uint16 y = 0; y < temp.h; y++)
		memcpy(temp.getBasePtr(0, y), surface->getBasePtr(cursor * 16, y), temp.w * 2);

	surface->free();
	delete surface;

	// Set the cursor using the image and the hotspot table
	CursorMan.replaceCursor(temp.getPixels(), 16, 17, hotspots[cursor][0], hotspots[cursor][1], 0, false, &temp.format);

	temp.free();
}

} // End of namespace MystJaguar
