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

#include "common/file.h"
#include "common/util.h"
#include "common/textconsole.h"

#include "sword1/sword1.h"
#include "sword1/music.h"

#include "audio/mixer.h"
#include "audio/audiostream.h"
#include "audio/decoders/aiff.h"
#include "audio/decoders/flac.h"
#include "audio/decoders/mp3.h"
#include "audio/decoders/vorbis.h"
#include "audio/decoders/wave.h"
#include "audio/decoders/xa.h"

#define SMP_BUFSIZE 8192

namespace Sword1 {

// This means fading takes 3 seconds.
#define FADE_LENGTH 3

// These functions are only called from Music, so I'm just going to
// assume that if locking is needed it has already been taken care of.

bool MusicHandle::play(const Common::String &filename, bool loop) {
	stop();

	// FIXME: How about using AudioStream::openStreamFile instead of the code below?
	// I.e.:
	//_audioSource = Audio::AudioStream::openStreamFile(fileBase, 0, 0, loop ? 0 : 1);

	Audio::RewindableAudioStream *stream = 0;

#ifdef USE_FLAC
	if (!stream) {
		if (_file.open(filename + ".flac")) {
			stream = Audio::makeFLACStream(&_file, DisposeAfterUse::NO);
			if (!stream)
				_file.close();
		}
	}

	if (!stream) {
		if (_file.open(filename + ".fla")) {
			stream = Audio::makeFLACStream(&_file, DisposeAfterUse::NO);
			if (!stream)
				_file.close();
		}
	}
#endif
#ifdef USE_VORBIS
	if (!stream) {
		if (_file.open(filename + ".ogg")) {
			stream = Audio::makeVorbisStream(&_file, DisposeAfterUse::NO);
			if (!stream)
				_file.close();
		}
	}
#endif
#ifdef USE_MAD
	if (!stream) {
		if (_file.open(filename + ".mp3")) {
			stream = Audio::makeMP3Stream(&_file, DisposeAfterUse::NO);
			if (!stream)
				_file.close();
		}
	}
#endif
	if (!stream) {
		if (_file.open(filename + ".wav"))
			stream = Audio::makeWAVStream(&_file, DisposeAfterUse::NO);
	}

	if (!stream) {
		if (_file.open(filename + ".aif"))
			stream = Audio::makeAIFFStream(&_file, DisposeAfterUse::NO);
	}

	if (!stream)
		return false;

	_audioSource = Audio::makeLoopingAudioStream(stream, loop ? 0 : 1);

	g_system->getMixer()->playStream(Audio::Mixer::kPlainSoundType, &_handle, this, -1, _volume, _pan, DisposeAfterUse::NO);
	fadeUp();

	return true;
}

bool MusicHandle::playPSX(uint16 id, bool loop) {
	stop();

	if (!_file.isOpen())
		if (!_file.open("tunes.dat"))
			return false;

	Common::File tableFile;
	if (!tableFile.open("tunes.tab"))
		return false;

	tableFile.seek((id - 1) * 8, SEEK_SET);
	uint32 offset = tableFile.readUint32LE() * 0x800;
	uint32 size = tableFile.readUint32LE();

	tableFile.close();

	// Because of broken tunes.dat/tab in psx demo, also check that tune offset is
	// not over file size
	if ((size != 0) && (size != 0xffffffff) && ((int32)(offset + size) <= _file.size())) {
		_file.seek(offset, SEEK_SET);
		_audioSource = Audio::makeLoopingAudioStream(Audio::makeXAStream(_file.readStream(size), 11025), loop ? 0 : 1);
		g_system->getMixer()->playStream(Audio::Mixer::kPlainSoundType, &_handle, this, -1, _volume, _pan, DisposeAfterUse::NO);
		fadeUp();
	} else {
		_audioSource = NULL;
		return false;
	}

	return true;
}

void MusicHandle::fadeDown() {
	if (streaming()) {
		Common::StackLock lock(_mutex);
		if (_fading < 0)
			_fading = -_fading;
		else if (_fading == 0)
			_fading = FADE_LENGTH * getRate();
		_fadeSamples = FADE_LENGTH * getRate();
	}
}

void MusicHandle::fadeUp() {
	if (streaming()) {
		Common::StackLock lock(_mutex);
		if (_fading > 0)
			_fading = -_fading;
		else if (_fading == 0)
			_fading = -1;
		_fadeSamples = FADE_LENGTH * getRate();
	}
}

bool MusicHandle::endOfData() const {
	Common::StackLock lock(_mutex);
	return !_audioSource || _audioSource->endOfData();
}

bool MusicHandle::endOfStream() const {
	Common::StackLock lock(_mutex);
	return !_audioSource || _audioSource->endOfStream();
}

bool MusicHandle::streaming() const {
	return g_system->getMixer()->isSoundHandleActive(_handle);
}

// if we don't have an audiosource, return some dummy values
// (There should be a better solution than this)
uint MusicHandle::getChannels() const {
	Common::StackLock lock(_mutex);
	return _audioSource ? _audioSource->getChannels() : 1;
}

int MusicHandle::getRate() const {
	Common::StackLock lock(_mutex);
	return (_audioSource) ? _audioSource->getRate() : 11025;
}

int MusicHandle::readBuffer(int16 *buffer, const int numSamples) {
	int totalSamples = 0;
	int16 *bufStart = buffer;
	if (!_audioSource)
		return 0;
	int expectedSamples = numSamples;
	while ((expectedSamples > 0) && _audioSource) { // _audioSource becomes NULL if we reach EOF and aren't looping
		int samplesReturned = _audioSource->readBuffer(buffer, expectedSamples);
		buffer += samplesReturned;
		totalSamples += samplesReturned;
		expectedSamples -= samplesReturned;
		if ((expectedSamples > 0) && _audioSource->endOfData()) {
			debug(2, "Music reached EOF");
			stop();
		}
	}

	Common::StackLock lock(_mutex);
	// buffer was filled, now do the fading (if necessary)
	int samplePos = 0;
	while ((_fading > 0) && (samplePos < totalSamples)) { // fade down
		--_fading;
		bufStart[samplePos] = (bufStart[samplePos] * _fading) / _fadeSamples;
		samplePos++;
		if (_fading == 0) {
			stop();
			// clear the rest of the buffer
			memset(bufStart + samplePos, 0, (totalSamples - samplePos) * 2);
			return samplePos;
		}
	}
	while ((_fading < 0) && (samplePos < totalSamples)) { // fade up
		bufStart[samplePos] = -(bufStart[samplePos] * --_fading) / _fadeSamples;
		if (_fading <= -_fadeSamples)
			_fading = 0;
	}
	return totalSamples;
}

void MusicHandle::stop() {
	// Lock the mutex until the stream ends
	Common::StackLock lock(_mutex);
	g_system->getMixer()->stopHandle(_handle);
	delete _audioSource;
	_audioSource = NULL;
	_file.close();
	_fading = 0;
}

void MusicHandle::setVolume(uint8 volL, uint8 volR) {
	_pan = (volR - volL) / 2;
	_volume = CLIP<int>((volR + volL) / 2, -127, 127);

	if (streaming()) {
		g_system->getMixer()->setChannelVolume(_handle, _volume);
		g_system->getMixer()->setChannelBalance(_handle, _pan);
	}
}

Music::Music(Audio::Mixer *pMixer) {
	_mixer = pMixer;
	_volumeL = _volumeR = 192;
}

Music::~Music() {
	_handles[0].stop();
	_handles[1].stop();
}

void Music::setVolume(uint8 volL, uint8 volR) {
	_volumeL = volL;
	_volumeR = volR;
	_handles[0].setVolume(volL, volR);
	_handles[1].setVolume(volL, volR);
}

void Music::giveVolume(uint8 *volL, uint8 *volR) {
	*volL = _volumeL;
	*volR = _volumeR;
}

void Music::startMusic(int32 tuneId, int32 loopFlag) {
	if (strlen(_tuneList[tuneId]) > 0) {
		int newStream = 0;
		if (_handles[0].streaming() && _handles[1].streaming()) {
			int streamToStop;
			// Both streams playing - one must be forced to stop.
			if (!_handles[0].fading() && !_handles[1].fading()) {
				// None of them are fading. Shouldn't happen,
				// so it doesn't matter which one we pick.
				streamToStop = 0;
			} else if (_handles[0].fading() && !_handles[1].fading()) {
				// Stream 0 is fading, so pick that one.
				streamToStop = 0;
			} else if (!_handles[0].fading() && _handles[1].fading()) {
				// Stream 1 is fading, so pick that one.
				streamToStop = 1;
			} else {
				// Both streams are fading. Pick the one that
				// is closest to silent.
				if (ABS(_handles[0].fading()) < ABS(_handles[1].fading()))
					streamToStop = 0;
				else
					streamToStop = 1;
			}
			_handles[streamToStop].stop();
		}
		if (_handles[0].streaming()) {
			_handles[0].fadeDown();
			newStream = 1;
		} else if (_handles[1].streaming()) {
			_handles[1].fadeDown();
			newStream = 0;
		}

		if (SwordEngine::isPsx()) {
			_handles[newStream].playPSX(tuneId, loopFlag != 0);
		} else if (!_handles[newStream].play(_tuneList[tuneId], loopFlag != 0)) {
			if (tuneId != 81) // file 81 was apparently removed from BS.
				warning("Can't find music file %s", _tuneList[tuneId]);
		}
	} else {
		if (_handles[0].streaming())
			_handles[0].fadeDown();
		if (_handles[1].streaming())
			_handles[1].fadeDown();
	}
}

void Music::fadeDown() {
	for (int i = 0; i < ARRAYSIZE(_handles); i++)
		if (_handles[i].streaming())
			_handles[i].fadeDown();
}

} // End of namespace Sword1
