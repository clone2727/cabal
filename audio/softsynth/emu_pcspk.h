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

// Based on the ScummVM (GPLv2+) emulated PC Speaker code (originally at audio/softsynth/pcspk.h)

#ifndef AUDIO_SOFTSYNTH_PCSPK_H
#define AUDIO_SOFTSYNTH_PCSPK_H

#include "audio/audiostream.h"
#include "audio/audiodev/pcspk.h"
#include "common/mutex.h"

namespace Audio {

class PCSpeaker : public AudioStream {
public:
	PCSpeaker();
	~PCSpeaker();

	/** Play a note for length ms.
	 *
	 *  If length is negative, play until told to stop.
	 */
	void play(int freq, int32 length);
	/** Stop the currently playing note after delay ms. */
	void stop(int32 delay = 0);
	/** Adjust the volume. */
	void setVolume(byte volume);

	bool isPlaying() const;

	int readBuffer(int16 *buffer, const int numSamples);

	bool isStereo() const	{ return false; }
	bool endOfData() const	{ return false; }
	bool endOfStream() const { return false; }
	int getRate() const;

protected:
	Common::Mutex _mutex;

	int _rate;
	bool _playForever;
	uint32 _oscLength;
	uint32 _oscSamples;
	uint32 _remainingSamples;
	uint32 _mixedSamples;
	byte _volume;

	static int8 generateWave(uint32 x, uint32 oscLength);
};

/**
 * An emulated PC speaker
 */
class EmulatedPCSpeaker : public virtual PCSpeakerDevice, protected virtual EmulatedAudioDevice {
public:
	EmulatedPCSpeaker();
	~EmulatedPCSpeaker();

	// AudioDevice API
	bool init();
	void reset();

	// PCSpeakerDevice API
	void startOutput(int freq);
	void startOutputTicks(int ticks);
	void stopOutput();

	// AudioStream API
	bool isStereo() const { return false; }

protected:
	// EmulatedAudioDevice API
	void generateSamples(int16 *buffer, int numSamples);

private:
	int _oscLength;
	int _oscPos;
};

} // End of namespace Audio

#endif // AUDIO_SOFTSYNTH_PCSPEAKER_H
