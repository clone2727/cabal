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

// Additional copyright for this file:
// Copyright (C) 1995 Presto Studios, Inc.

#include "buried/avi_frames.h"
#include "buried/buried.h"
#include "buried/graphics.h"

#include "common/system.h"
#include "graphics/surface.h"
#include "video/avi_decoder.h"

namespace Buried {

AVIFrames::AVIFrames(const Common::String &fileName, uint cachedFrames) {
	_maxCachedFrames = 0;
	_video = 0;
	_cacheEnabled = false;
	_lastFrame = 0;
	_lastFrameIndex = -1;
	_tempFrame = 0;

	if (!fileName.empty())
		open(fileName, cachedFrames);
}

AVIFrames::~AVIFrames() {
	close();
}

bool AVIFrames::open(const Common::String &fileName, uint cachedFrames) {
	if (fileName.empty())
		return false;

	if (_fileName == fileName)
		return true;

	close();

	_video = new Video::AVIDecoder();

	if (!_video->loadFile(fileName)) {
		close();
		return false;
	}

	_fileName = fileName;

	// Put us into dither mode, for the 8bpp version
	BuriedEngine *vm = (BuriedEngine *)g_engine;
	if (!vm->isTrueColor())
		_video->setDitheringPalette(vm->_gfx->getDefaultPalette());

	if (cachedFrames == 0) {
		_cacheEnabled = false;
	} else {
		_maxCachedFrames = cachedFrames;
		_cacheEnabled = true;
	}

	_lastFrameIndex = -1;
	return true;
}

void AVIFrames::close() {
	delete _video;
	_video = 0;

	_fileName.clear();

	flushFrameCache();

	_lastFrameIndex = -1;
	_lastFrame = 0;

	if (_tempFrame) {
		_tempFrame->free();
		delete _tempFrame;
		_tempFrame = 0;
	}
}

const Graphics::Surface *AVIFrames::getFrame(int frameIndex) {
	if (!_video)
		return 0;

	if (frameIndex < 0 || frameIndex == _lastFrameIndex)
		return _lastFrame;

	if (_cacheEnabled) {
		const Graphics::Surface *cachedFrame = retrieveFrameFromCache(frameIndex);
		if (cachedFrame)
			return cachedFrame;
	}

	if (!_video->seekToFrame(frameIndex))
		return 0;

	const Graphics::Surface *frame = _video->decodeNextFrame();
	if (!frame)
		return 0;

	Graphics::Surface *copy;
	if (frame->format == g_system->getScreenFormat()) {
		copy = new Graphics::Surface();
		copy->copyFrom(*frame);
	} else {
		copy = frame->convertTo(g_system->getScreenFormat());
	}

	if (_cacheEnabled) {
		addFrameToCache(frameIndex, copy);
	} else {
		if (_tempFrame) {
			_tempFrame->free();
			delete _tempFrame;
		}

		_tempFrame = copy;
	}

	return copy;
}

Graphics::Surface *AVIFrames::getFrameCopy(int frameIndex) {
	const Graphics::Surface *frame = getFrame(frameIndex);
	if (!frame)
		return 0;

	Graphics::Surface *copy = new Graphics::Surface();
	copy->copyFrom(*frame);
	return copy;
}

int AVIFrames::getFrameCount() {
	if (!_video)
		return 0;

	return _video->getFrameCount();
}

bool AVIFrames::flushFrameCache() {
	if (_cachedFrames.empty())
		return false;

	for (FrameList::iterator it = _cachedFrames.begin(); it != _cachedFrames.end(); it++) {
		if (it->frame) {
			it->frame->free();
			delete it->frame;
		}
	}

	return true;
}

const Graphics::Surface *AVIFrames::retrieveFrameFromCache(int frameIndex) const {
	for (FrameList::const_iterator it = _cachedFrames.begin(); it != _cachedFrames.end(); it++)
		if (it->index == frameIndex)
			return it->frame;

	return 0;
}

void AVIFrames::addFrameToCache(int frameIndex, Graphics::Surface *frame) {
	if (_cachedFrames.size() >= _maxCachedFrames) {
		CachedFrame &cachedFrame = _cachedFrames.front();
		if (cachedFrame.frame) {
			cachedFrame.frame->free();
			delete cachedFrame.frame;
		}
		
		_cachedFrames.pop_front();
	}

	_cachedFrames.push_back(CachedFrame(frameIndex, frame));
}

} // End of namespace Buried
