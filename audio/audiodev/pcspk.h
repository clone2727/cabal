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

#ifndef AUDIO_AUDIODEV_PCSPK_H
#define AUDIO_AUDIODEV_PCSPK_H

#include "audio/audiodev/audiodev.h"

namespace Audio {

/**
 * An abstract PC speaker
 */
class PCSpeakerDevice : public virtual AudioDevice {
public:
	/**
	 * Begin playback at the given frequency
	 */
	virtual void startOutput(int freq) = 0;

	/**
	 * Begin playback with the given ticks
	 */
	virtual void startOutputTicks(int ticks) = 0;

	/**
	 * Stop playing the note
	 */
	virtual void stopOutput() = 0;

	/**
	 * The frequency of the device
	 */
	static const int kDeviceFreq = 1193180;
};

} // End of namespace Audio

#endif

