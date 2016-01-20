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

#ifndef AUDIO_AUDIODEV_AUDIODEV_H
#define AUDIO_AUDIODEV_AUDIODEV_H

#include "audio/audiostream.h"
#include "common/func.h"

namespace Audio {

class SoundHandle;

/**
 * An abstract sound device
 */
class AudioDevice {
public:
	virtual ~AudioDevice() {}

	/**
	 * The type of the timer callback functor.
 	 */
	typedef Common::Functor0<void> TimerCallback;

	/**
	 * Initializes the device.
	 *
	 * @return true on success, false on failure
	 */
	virtual bool init() = 0;

	/**
	 * Reinitializes the device
	 */
	virtual void reset() = 0;

	/**
	 * Start the OPL with callbacks.
	 */
	void start(TimerCallback *callback, int timerFrequency = kDefaultCallbackFrequency);

	/**
	 * Stop the OPL
	 */
	void stop();

	/**
	 * Change the callback frequency. This must only be called from a
	 * timer proc.
	 */
	virtual void setCallbackFrequency(int timerFrequency) = 0;

	enum {
		/**
		 * The default callback frequency that start() uses
		 */
		kDefaultCallbackFrequency = 250
	};

protected:
	/**
	 * Start the callbacks.
	 */
	virtual void startCallbacks(int timerFrequency) = 0;

	/**
	 * Stop the callbacks.
	 */
	virtual void stopCallbacks() = 0;

	/**
	 * The functor for callbacks.
	 */
	Common::ScopedPtr<TimerCallback> _callback;
};

/**
 * An abstract sound device that is for a real hardware device. Uses
 * the TimerManager for callbacks.
 */
class HardwareAudioDevice : public virtual AudioDevice {
public:
	HardwareAudioDevice();
	virtual ~HardwareAudioDevice();

	// AudioDevice API
	void setCallbackFrequency(int timerFrequency);

protected:
	// AudioDevice API
	void startCallbacks(int timerFrequency);
	void stopCallbacks();

private:
	static void timerProc(void *refCon);
	void onTimer();

	uint _baseFreq;
	uint _remainingTicks;

	enum {
		kMaxFreq = 100
	};
};

/**
 * An abstract sound device that represents an emulated device. Uses
 * the Mixer playback for callbacks.
 */
class EmulatedAudioDevice : public virtual AudioDevice, protected Audio::AudioStream {
public:
	EmulatedAudioDevice();
	virtual ~EmulatedAudioDevice();

	// AudioDevice API
	void setCallbackFrequency(int timerFrequency);

	// AudioStream API
	int readBuffer(int16 *buffer, const int numSamples);
	int getRate() const;
	bool endOfData() const { return false; }

protected:
	// AudioDevice API
	void startCallbacks(int timerFrequency);
	void stopCallbacks();

	/**
	 * Read up to 'length' samples.
	 *
	 * Data will be in native endianess, 16 bit per sample, signed.
	 * For a stereo device, buffer will be filled with interleaved
	 * left and right channel samples, starting with a left sample.
	 * Furthermore, the samples in the left and right are summed up.
	 * So if you request 4 samples from a stereo device, you will get
	 * a total of two left channel and two right channel samples.
	 */
	virtual void generateSamples(int16 *buffer, int numSamples) = 0;

private:
	int _baseFreq;

	enum {
		FIXP_SHIFT = 16
	};

	int _nextTick;
	int _samplesPerTick;

	Audio::SoundHandle *_handle;
};

} // End of namespace Audio

#endif
