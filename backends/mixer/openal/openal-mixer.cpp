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

// Very very loosely based on the xoreos SoundManager code
// Used with permission of clone2727 and DrMcCoy

// Mac OS X is special because of frameworks
#ifdef MACOSX
	#include <OpenAL/al.h>
	#include <OpenAL/alc.h>
	#include <OpenAL/MacOSX_OALExtensions.h>
#else
	#include <AL/al.h>
	#include <AL/alc.h>
#endif

#include "common/scummsys.h"

#include "audio/mixer_intern.h"
#include "backends/mixer/mixer_manager.h"
#include "backends/mixer/openal/openal-mixer.h"
#include "common/debug.h"
#include "common/queue.h"
#include "common/system.h"
#include "common/timer.h"
#include "common/config-manager.h"
#include "common/textconsole.h"

class OpenALMixerManager : public MixerManager {
public:
	OpenALMixerManager();
	~OpenALMixerManager();

	/**
	 * Initialize and setups the mixer
	 */
	void init();

	/**
	 * Get the audio mixer implementation
	 */
	Audio::Mixer *getMixer() const { return (Audio::Mixer *)_mixer; }

	/**
	 * Update the OpenAL source from the mixer.
	 */
	void update();

private:
	/** The mixer implementation */
	Audio::MixerImpl *_mixer;

	// OpenAL context
	ALCdevice *_dev;
	ALCcontext *_ctx;

	enum {
		kBufferCount = 5,
		kBufferSize = 32768
	};

	// OpenAL source/buffer
	ALuint _source;
	ALuint _buffers[kBufferCount];
	Common::Queue<ALuint> _freeBuffers;

	static void timerCallback(void *refCon) {
		OpenALMixerManager *mixerManager = (OpenALMixerManager *)refCon;
		mixerManager->update();
	}

	uint32 getOpenALOutputRate() const;
};

OpenALMixerManager::OpenALMixerManager()
	:
	_mixer(0),
	_dev(0),
	_ctx(0) {

}

OpenALMixerManager::~OpenALMixerManager() {
	if (_mixer) {
		g_system->getTimerManager()->removeTimerProc(timerCallback);

		_mixer->setReady(false);

		// Stop the source
		alSourceStop(_source);
		alDeleteSources(1, &_source);

		// Free the buffers
		alDeleteBuffers(5, _buffers);

		// And destroy the context too
		alcMakeContextCurrent(0);
		alcDestroyContext(_ctx);
		alcCloseDevice(_dev);

		delete _mixer;
	}
}

#define CHECK_AL_ERROR() \
	do { \
		ALenum errorCode = alGetError(); \
		if (errorCode != AL_NO_ERROR) \
			error("OpenAL error: %X", errorCode); \
	} while (0)

void OpenALMixerManager::init() {
	_dev = alcOpenDevice(0);

	if (!_dev)
		error("Could not open OpenAL device");

	_ctx = alcCreateContext(_dev, 0);

	if (!_ctx)
		error("Could not create OpenAL context");

	alcMakeContextCurrent(_ctx);

	// Put the listener at full volume -- MixerImpl handles volume changes
	alListenerf(AL_GAIN, 1.0f);

	alGenSources(1, &_source);
	CHECK_AL_ERROR();

	// Put the source at full volume -- MixerImpl handles volume changes
	alSourcef(_source, AL_GAIN, 1.0f);

	// Generate buffers for our source
	alGenBuffers(kBufferCount, _buffers);
	CHECK_AL_ERROR();

	// Add all our buffers to our free queue
	for (uint i = 0; i < kBufferCount; i++)
		_freeBuffers.push(_buffers[i]);

	_mixer = new Audio::MixerImpl(g_system, getOpenALOutputRate());
	_mixer->setReady(true);

	// Queue a buffer to start us off
	update();

	// Then play the source
	alSourcePlay(_source);
	CHECK_AL_ERROR();

	g_system->getTimerManager()->installTimerProc(timerCallback, 1000 * 100, this, "OpenALMixerManager");
}

void OpenALMixerManager::update() {
	// Unqueue any buffers that have been processed
	ALint buffersProcessed;
	alGetSourcei(_source, AL_BUFFERS_PROCESSED, &buffersProcessed);

	while (buffersProcessed--) {
		ALuint alBuffer;
		alSourceUnqueueBuffers(_source, 1, &alBuffer);
		_freeBuffers.push(alBuffer);
	}

	// Don't buffer more if we have enough buffered already
	if (_freeBuffers.empty())
		return;

	// Fill a buffer
	byte *buffer = new byte[kBufferSize];
	_mixer->mixCallback(buffer, kBufferSize);

	ALuint alBuffer = _freeBuffers.pop();
	alBufferData(alBuffer, AL_FORMAT_STEREO16, buffer, kBufferSize, _mixer->getOutputRate());
	delete[] buffer;

	// Now, queue it
	alSourceQueueBuffers(_source, 1, &alBuffer);

	// Ensure we're still playing
	ALint sourceState;
	alGetSourcei(_source, AL_SOURCE_STATE, &sourceState);
	if (sourceState != AL_PLAYING)
		alSourcePlay(_source);
}

uint32 OpenALMixerManager::getOpenALOutputRate() const {
#ifdef MACOSX
	// alcGetIntegerv on ALC_FREQUENCY seems not to work on OS X
	// Use this method to get the frequency instead.

	// Workaround C++ not liking assigning function pointers from void pointers
	union {
		void *voidPtr;
		alcMacOSXGetMixerOutputRateProcPtr procPtr;
	} temp;

	temp.voidPtr = alcGetProcAddress(0, (const ALCchar *)"alcMacOSXGetMixerOutputRate");
	alcMacOSXGetMixerOutputRateProcPtr proc = temp.procPtr;

	if (proc)
		return (uint32)proc();
#else
	ALint outputRate;
	alcGetIntegerv(_dev, ALC_FREQUENCY, 1, &outputRate);

	if (alcGetError(_dev) == ALC_NO_ERROR)
		return outputRate;
#endif

	// Default (what most devices are probably outputting anyway)
	return 44100;
}

MixerManager *createOpenALMixerManager() {
	return new OpenALMixerManager();
}
