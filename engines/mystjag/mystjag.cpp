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

#include "common/error.h"
#include "common/events.h"
#include "common/stream.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/surface.h"
#include "video/segafilm_decoder.h"

#include "mystjag/graphics.h"
#include "mystjag/mystjag.h"
#include "mystjag/session.h"
#include "mystjag/sound.h"

namespace MystJaguar {

MystJaguarEngine::MystJaguarEngine(OSystem *syst, const MystJaguarGameDescription *gamedesc) : Engine(syst), _gameDescription(gamedesc) {
	_gfx = 0;
	_sound = 0;
	_session = 0;
}

MystJaguarEngine::~MystJaguarEngine() {
	delete _gfx;
	delete _sound;
	delete _session;
}

Common::Error MystJaguarEngine::run() {
	_gfx = new GraphicsManager();
	_sound = new SoundManager();
	_session = new SessionManager(isDemo());

	// Initialize graphics
	initGraphics(544, 384, true, 0);

	// I mean, we should have high color since we requested it in configure...
	if (g_system->getScreenFormat().bytesPerPixel == 1)
		return Common::kUnsupportedColorMode;

	// Load the offset table
	_session->loadOffsetTable();

	// Show off the Cyan logo
	Common::SeekableReadStream *cyanLogoStream = _session->getFile(0, isDemo() ? 0 : 2);
	Video::SegaFILMDecoder video;
	if (!video.loadStream(cyanLogoStream))
		error("Failed to load Cyan logo video");

	uint32 x = (_system->getWidth() - video.getWidth()) / 2;
	uint32 y = (_system->getHeight() - video.getHeight()) / 2;

	video.start();

	while (!shouldQuit() && !video.endOfVideo()) {
		if (video.needsUpdate()) {
			const Graphics::Surface *frame = video.decodeNextFrame();

			if (frame) {
				_system->copyRectToScreen(frame->getPixels(), frame->pitch, x, y, frame->w, frame->h);
				_system->updateScreen();
			}
		}

		Common::Event event;
		while (_eventMan->pollEvent(event))
			;

		_system->delayMillis(10);
	}
	

	return Common::kNoError;
}

} // End of namespace MystJaguar
