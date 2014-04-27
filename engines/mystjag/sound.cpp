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

#include "audio/decoders/raw.h"
#include "common/array.h"
#include "common/memstream.h"

#include "mystjag/sound.h"

namespace MystJaguar {

SoundManager::SoundManager() {
}

SoundManager::~SoundManager() {
}

struct ChunkTableEntry {
	uint32 offset;
	uint32 size;
	uint32 syncBytes; // Not really important for us (repeated 16 times before each chunk)
};

Audio::SeekableAudioStream *SoundManager::decodeJSND(Common::SeekableReadStream &stream) {
	// Main header
	uint32 tag = stream.readUint32BE();
	if (tag != MKTAG('J', 'S', 'N', 'D'))
		error("Failed to find sound header");

	uint32 jsndSize = stream.readUint32BE();
	stream.readUint32BE(); // unknown (always 0)
	stream.readUint32BE(); // unknown (always 0)

	// Sound description
	tag = stream.readUint32BE();
	if (tag != MKTAG('S', 'D', 'S', 'C'))
		error("Failed to find sound description");

	stream.readUint32BE(); // broken SDSC size (always 16, should be 20)
	uint32 sampleRate = stream.readUint32BE();
	stream.readUint32BE(); // unknown
	stream.readUint32BE(); // always 0?

	// Chunk table
	tag = stream.readUint32BE();
	if (tag != MKTAG('C', 'T', 'A', 'B'))
		error("Failed to find sound chunk table");

	stream.readUint32BE(); // CTAB size
	uint32 chunkCount = stream.readUint32BE();

	Common::Array<ChunkTableEntry> chunks;
	chunks.resize(chunkCount);
	uint32 totalSize = 0;

	for (uint32 i = 0; i < chunkCount; i++) {
		ChunkTableEntry &entry = chunks[i];
		entry.offset = stream.readUint32BE() + jsndSize;
		entry.size = stream.readUint32BE();
		entry.syncBytes = stream.readUint32BE();
		totalSize += entry.size;
	}

	// Allocate and read the data
	byte *soundData = (byte *)malloc(totalSize);
	byte *soundPtr = soundData;

	for (uint32 i = 0; i < chunkCount; i++) {
		ChunkTableEntry &entry = chunks[i];
		stream.seek(entry.offset);
		stream.read(soundPtr, entry.size);
		soundPtr += entry.size;
	}

	return Audio::makeRawStream(soundData, totalSize, sampleRate, 0);
}

} // End of namespace MystJaguar
