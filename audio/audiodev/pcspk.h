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
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/singleton.h"
#include "common/str.h"

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

/**
 * An abstract hardware PC speaker
 */
class HardwarePCSpeaker : public virtual PCSpeakerDevice, protected virtual HardwareAudioDevice {
};

/**
 * Factory manager for PC speaker objects
 */
class PCSpeakerFactoryManager : public Common::Singleton<PCSpeakerFactoryManager> {
public:
	/**
	 * Create a new PCSpeakerDevice based on configuration and what factories
	 * are available.
	 */
	PCSpeakerDevice *createDevice();

	/**
	 * A function that returns a PCSpeakerDevice and takes no parameters
	 */
	typedef PCSpeakerDevice *(*CreateDeviceFunc)();

	/**
	 * Register a new PC speaker device factory
	 */
	void registerFactory(const Common::String &name, const Common::String &description, CreateDeviceFunc factory);

private:
	friend class Common::Singleton<SingletonBaseType>;
	PCSpeakerFactoryManager();

	struct Factory {
		Common::String description;
		CreateDeviceFunc factory;
	};

	typedef Common::HashMap<Common::String, Factory> FactoryMap;
	FactoryMap _factories;
};

} // End of namespace Audio

/** Shortcut for accessing the PC speaker factory manager. */
#define PCSpeakerFactoryMan ::Audio::PCSpeakerFactoryManager::instance()

#endif

