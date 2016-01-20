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

#include "audio/audiodev/pcspk.h"
#include "audio/softsynth/emu_pcspk.h"
#include "common/config-manager.h"

namespace Common {
DECLARE_SINGLETON(Audio::PCSpeakerFactoryManager);
}

namespace Audio {

PCSpeakerFactoryManager::PCSpeakerFactoryManager() {
	// Register the default
	registerFactory("emu", "Software Emulation", &createEmulatedPCSpeaker);
}

PCSpeakerDevice *PCSpeakerFactoryManager::createDevice() {
	Common::String deviceName = ConfMan.get("pcspk_dev");
	deviceName.toLowercase();

	// Default to emulated
	if (deviceName.empty() || deviceName == "auto")
		deviceName = "emu";

	FactoryMap::iterator it = _factories.find(deviceName);
	if (it == _factories.end()) {
		// Default to the emulated version
		warning("'%s' is not a valid PC speaker device; defaulting to emulated", deviceName.c_str());
		return createEmulatedPCSpeaker();
	}

	return it->_value.factory();
}

void PCSpeakerFactoryManager::registerFactory(const Common::String &name, const Common::String &description, CreateDeviceFunc factory) {
	Factory newFactory;
	newFactory.description = description;
	newFactory.factory = factory;
	_factories[name] = newFactory;
}

} // End of namespace Audio

