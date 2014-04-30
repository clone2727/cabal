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

	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember("mystjag2.dat");

	if (!stream)
		return false;

	// Demo offset table
	stream->seek(0xAF22);
	uint32 offset = 0;

	for (int i = 0; i < 400; i++) {
		FileEntry entry;
		entry.unk = stream->readUint16BE();
		entry.sector = stream->readUint32BE();
		entry.size = stream->readUint32BE();
		entry.syncBytes = stream->readUint32BE();
		entry.offset = offset;
		offset += entry.size + 0x40;
		_files.push_back(entry);
	}

	delete stream;
	return true;
}

Common::SeekableReadStream *SessionManager::getFile(uint file) {
	// Get the file from the data track

	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember("mystjag3.dat");

	if (!stream)
		error("Failed to open mystjag3.dat");

	if (file >= _files.size())
		error("Invalid file %d", file);

	const FileEntry &entry = _files[file];
	uint32 offset = entry.offset + 0x4EA;
	return new Common::SeekableSubReadStream(stream, offset, offset + entry.size, DisposeAfterUse::YES);
}


} // End of namespace MystJaguar
