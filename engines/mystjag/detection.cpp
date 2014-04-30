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

#include "base/plugins.h"

#include "engines/advancedDetector.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/savefile.h"

#include "mystjag/mystjag.h"

namespace MystJaguar {

struct MystJaguarGameDescription {
	ADGameDescription desc;
};

bool MystJaguarEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL);
}

bool MystJaguarEngine::isDemo() const {
	return (_gameDescription->desc.flags & ADGF_DEMO) != 0;
}

} // End of namespace MystJaguar

static const PlainGameDescriptor mystJaguarGames[] = {
	{"mystjag", "Myst"},
	{0, 0}
};


namespace MystJaguar {

static const MystJaguarGameDescription gameDescriptions[] = {
	{
		{
			"mystjag",
			"Demo",
			AD_ENTRY1s("mystjag3.dat", "9bf4d4487b5b3b9f67edc5e10a8ce67a", 62360928),
			Common::EN_ANY,
			Common::kPlatformJaguar,
			ADGF_DEMO,
			GUIO0()
		},
	},

	{ AD_TABLE_END_MARKER }
};

} // End of namespace MystJaguar


class MystJaguarMetaEngine : public AdvancedMetaEngine {
public:
	MystJaguarMetaEngine() : AdvancedMetaEngine(MystJaguar::gameDescriptions, sizeof(MystJaguar::MystJaguarGameDescription), mystJaguarGames) {
		_singleid = "mystjag";
	}

	virtual const char *getName() const {
		return "Myst (Atari Jaguar)";
	}

	virtual const char *getOriginalCopyright() const {
		return "Myst (Atari Jaguar) (C) Cyan Worlds, Sunsoft, Atari";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
};

bool MystJaguarMetaEngine::hasFeature(MetaEngineFeature f) const {
	return false;
}

bool MystJaguarMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const MystJaguar::MystJaguarGameDescription *gd = (const MystJaguar::MystJaguarGameDescription *)desc;

	if (gd)
		*engine = new MystJaguar::MystJaguarEngine(syst, gd);

	return (gd != 0);
}

#if PLUGIN_ENABLED_DYNAMIC(MYSTJAG)
	REGISTER_PLUGIN_DYNAMIC(MYSTJAG, PLUGIN_TYPE_ENGINE, MystJaguarMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(MYSTJAG, PLUGIN_TYPE_ENGINE, MystJaguarMetaEngine);
#endif

