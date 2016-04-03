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

#ifndef UNITY_SOUND_H
#define UNITY_SOUND_H

#include "unity.h"
#include "audio/mixer.h"

namespace Unity {

class Sound {
public:
	Sound(UnityEngine *engine);
	~Sound();

	void init();
	void playAudioBuffer(unsigned int length, byte *data);
	void playSfx(Common::String name);
	void playSpeech(Common::String name);
	void playMusic(Common::String name, byte volume = 0xff, int loopPos = -1);
	bool sfxPlaying();
	bool speechPlaying();
	bool musicPlaying();
	void stopSfx();
	void stopSpeech();
	void stopMusic();

	void playIntroMusic();
	void updateMusic();

protected:
	UnityEngine *_vm;
	Audio::SoundHandle *_sfxSoundHandle;
	Audio::SoundHandle *_speechSoundHandle;
	Audio::SoundHandle *_musicSoundHandle;
};

}

#endif

