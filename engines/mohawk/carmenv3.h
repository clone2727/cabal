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

#ifndef MOHAWK_CARMENV3_H
#define MOHAWK_CARMENV3_H

#include "mohawk/mohawk.h"

namespace Common {
class String;
}

namespace Mohawk {

struct MohawkGameDescription;
class Archive;

class MohawkEngine_CarmenV3 : public MohawkEngine {
public:
	MohawkEngine_CarmenV3(OSystem *syst, const MohawkGameDescription *gamedesc);
	virtual ~MohawkEngine_CarmenV3();

	Common::Error run();

	GUI::Debugger *getDebugger();

private:
	GUI::Debugger *_console;

	// Runtime files
	Archive *_mainArchive;
	Archive *_countryArchive;
	Archive *_dialogArchive;
	Archive *_guidesArchive;
	Common::Array<Archive *> _characterArchives;
	Archive *_amtrakArchive2;

	void playVideoCentered(const Common::String &fileName);
	void playIntro();
};

} // End of namespace Mohawk

#endif
