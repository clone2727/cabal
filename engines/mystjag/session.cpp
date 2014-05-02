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

bool SessionManager::loadOffsetTable() {
	// Load the file offset table from executable track

	// TODO: There's no way in hell the full table will be at the same offset
	if (!_isDemo)
		return false;

	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember("mystjag0.dat");

	if (!stream)
		return false;

	// Demo offset table
	stream->seek(0xAF22);

	for (int i = 0; i < 400; i++) {
		FileEntry entry;
		entry.track = stream->readUint16BE();
		entry.sector = stream->readUint32BE();
		entry.size = stream->readUint32BE();
		entry.syncBytes = stream->readUint32BE();
		_files.push_back(entry);
	}

	delete stream;
	return true;
}

enum {
	kSectorSize = 2352,
	kSectorCheckCount = 2 // Apparently the sectors aren't always correct. This works for the demo though.
};

Common::SeekableReadStream *SessionManager::getFile(uint file) {
	if (file >= _files.size())
		error("Invalid file %d", file);

	const FileEntry &entry = _files[file];
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
		error("Failed to find file %d (track %d, sector %d, sync '%s')", file, entry.track, entry.sector, tag2str(entry.syncBytes));

	return new Common::SeekableSubReadStream(stream, stream->pos(), stream->pos() + entry.size, DisposeAfterUse::YES);
}


} // End of namespace MystJaguar
