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

#include "common/archive.h"
#include "common/stream.h"
#include "common/substream.h"
#include "mystjag/session.h"

namespace MystJaguar {

SessionManager::SessionManager(bool isDemo) : _isDemo(isDemo) {
}

SessionManager::~SessionManager() {
}

struct OffsetTableLocation {
	uint32 offset;
	uint32 entryCount;
};

static const OffsetTableLocation s_demoOffsetTables[] = {
	{ 0xAF22, 400 } // Demo/Myst (track 1)
};

static const OffsetTableLocation s_fullOffsetTables[] = {
	{ 0x1FFB6, 1069 }, // Myst (track 2)
	{ 0x33E1A, 369 },  // Stoneship (track 3)
	{ 0x425BE, 351 },  // Selenitic (track 4)
	{ 0x51EA2, 326 },  // Mechanical (track 5)
	{ 0x68C26, 685 },  // Channelwood (track 6)
	{ 0x6CD8A, 57 }    // D'ni (track 7)
};

bool SessionManager::loadOffsetTable() {
	// Load the file offset tables from executable track

	// The full game uses a boot track to load the executables. The demo is wholly in the
	// boot track. There's an offset table for the executables in the boot track, but it's
	// completely not worth loading it.
	Common::String fileName = Common::String::format("mystjag%d.dat", _isDemo ? 0 : 1);
	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(fileName);

	if (!stream)
		return false;

	const OffsetTableLocation *locations = _isDemo ? s_demoOffsetTables : s_fullOffsetTables;
	uint locationCount = _isDemo ? ARRAYSIZE(s_demoOffsetTables) : ARRAYSIZE(s_fullOffsetTables);
	_fileTables.resize(locationCount);

	for (uint i = 0; i < locationCount; i++) {
		const OffsetTableLocation &location = locations[i];
		stream->seek(location.offset);

		FileTable &table = _fileTables[i];
		table.resize(location.entryCount);

		for (uint j = 0; j < location.entryCount; j++) {
			FileEntry &entry = table[j];
			entry.track = stream->readUint16BE();
			entry.sector = stream->readUint32BE();
			entry.size = stream->readUint32BE();
			entry.syncBytes = stream->readUint32BE();
		}
	}

	delete stream;
	return true;
}

enum {
	kSectorSize = 2352,
	kSectorCheckCount = 3 // Apparently the sectors aren't always correct. This works for the demo though.
};

Common::SeekableReadStream *SessionManager::getFile(uint stack, uint file) {
	if (stack >= _fileTables.size())
		error("Invalid stack %d", stack);

	const FileTable &table = _fileTables[stack];

	if (file >= table.size())
		error("Invalid file %d", file);

	const FileEntry &entry = table[file];
	Common::String fileName = Common::String::format("mystjag%d.dat", entry.track);
	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(fileName);

	if (!stream)
		error("Failed to open '%s'", fileName.c_str());

	// Get to the sector offset
	uint32 offset = entry.sector * kSectorSize;
	stream->seek(offset);

	// Now we get to search for the sync
	uint32 test = stream->readUint32BE();

	while ((uint32)stream->pos() < offset + kSectorSize * kSectorCheckCount) {
		if (test == entry.syncBytes) {
			uint32 curOffset = stream->pos();
			bool valid = true;

			// OK, now we need to find this 15 more times
			for (uint32 i = 0; i < 15; i++) {
				uint32 check = stream->readUint32BE();
				if (check != entry.syncBytes) {
					valid = false;
					break;
				}
			}

			if (valid) {
				// Yay, we found it
				break;
			} else {
				// Boo :(
				stream->seek(curOffset);
			}
		}

		// Shift and grab the next byte
		test <<= 8;
		test |= stream->readByte();
	}

	if ((uint32)stream->pos() == offset + kSectorSize * kSectorCheckCount)
		error("Failed to find file %d (stack %d, track %d, sector %d, sync '%s')", stack, file, entry.track, entry.sector, tag2str(entry.syncBytes));

	return new Common::SeekableSubReadStream(stream, stream->pos(), stream->pos() + entry.size, DisposeAfterUse::YES);
}


} // End of namespace MystJaguar
