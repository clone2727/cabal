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

#include "common/debug.h"
#include "common/error.h"
#include "common/system.h"

#include "gui/debugger.h"

#include "mohawk/carmenv3.h"
#include "mohawk/resource.h"

namespace Mohawk {

MohawkEngine_CarmenV3::MohawkEngine_CarmenV3(OSystem *syst, const MohawkGameDescription *gamedesc) : MohawkEngine(syst, gamedesc) {
	_console = 0;
	_mainArchive = 0;
	_countryArchive = 0;
	_dialogArchive = 0;
	_guidesArchive = 0;
	_amtrakArchive2 = 0;
}

MohawkEngine_CarmenV3::~MohawkEngine_CarmenV3() {
	delete _mainArchive;
	delete _countryArchive;
	delete _dialogArchive;
	delete _guidesArchive;
	delete _amtrakArchive2;

	for (uint32 i = 0; i < _characterArchives.size(); i++)
		delete _characterArchives[i];

	delete _console;
}

GUI::Debugger *MohawkEngine_CarmenV3::getDebugger() {
	return _console;
}

Common::Error MohawkEngine_CarmenV3::run() {
	// Initialize common functions
	MohawkEngine::run();

	// Create a dummy console
	_console = new GUI::Debugger();

	// TODO: Load the installer archive, if present
	if (getPlatform() == Common::kPlatformWindows) {
		// Windows crap (Z archive)
	} else {
		// Mac crap (Vise)
	}

	// Open the main archive
	_mainArchive = new MohawkArchive();
	if (getGameType() == GType_CSWORLD) {
		if (!_mainArchive->openFile("C2K.MHK"))
			error("Failed to open C2K.MHK");
	} else {
		if (!_mainArchive->openFile("USAC2K.MHK"))
			error("Failed to open USAC2K.MHK");
	}

	// CSAmtrak has an extra one
	if (getGameType() == GType_CSAMTRAK) {
		_amtrakArchive2 = new MohawkArchive();
		if (!_amtrakArchive2->openFile("AMTRAK2.MHK"))
			error("Failed to open AMTRAK2.MHK");
	}

	// Open the other common ones
	_countryArchive = new MohawkArchive();
	if (!_countryArchive->openFile("COUNTRY.MHK"))
		error("Failed to open COUNTRY.MHK");

	_dialogArchive = new MohawkArchive();
	if (!_dialogArchive->openFile("DIALOG.MHK"))
		error("Failed to open DIALOG.MHK");

	_guidesArchive = new MohawkArchive();
	if (!_guidesArchive->openFile("GUIDES.MHK"))
		error("Failed to open GUIDES.MHK");

	// Open all the character archives
	for (int i = 0; i < 8; i++) {
		MohawkArchive *archive = new MohawkArchive();
		Common::String fileName = Common::String::format("CHR%d.MHK", i + 1);
		if (!archive->openFile(fileName))
			error("Failed to open %s", fileName.c_str());
		_characterArchives.push_back(archive);
	}

	// TODO: Initialize at 640x480x[8|16]bpp

	return Common::kNoError;
}

} // End of namespace Mohawk
