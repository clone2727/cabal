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

#ifndef MYSTJAG_SESSION_H
#define MYSTJAG_SESSION_H

#include "common/array.h"
#include "common/scummsys.h"

namespace Common {
class SeekableReadStream;
}

namespace MystJaguar {

struct FileEntry {
	uint16 unk; // always 1?
	uint32 sector;
	uint32 size;
	uint32 syncBytes;
};

class SessionManager {
public:
	SessionManager(bool isDemo);
	~SessionManager();

	bool loadOffsetTable();
	Common::SeekableReadStream *getFile(uint file);

private:
	bool _isDemo;
	Common::Array<FileEntry> _files;
};

} // End of namespace MystJaguar

#endif
