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

// Based on the ScummVM (GPLv2+) file of the same name

#include "common/debug.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/str.h"
#include "common/stream.h"
#include "common/winexe_ne.h"

namespace Common {

NEResources::NEResources() {
	_exe = 0;
}

NEResources::~NEResources() {
	clear();
}

void NEResources::clear() {
	if (_exe) {
		delete _exe;
		_exe = 0;
	}

	_resources.clear();
}

bool NEResources::loadFromEXE(const String &fileName) {
	if (fileName.empty())
		return false;

	File *file = new File();

	if (!file->open(fileName)) {
		delete file;
		return false;
	}

	return loadFromEXE(file);
}

bool NEResources::loadFromEXE(SeekableReadStream *stream) {
	clear();

	if (!stream)
		return false;

	_exe = stream;

	uint32 offsetResourceTable = getResourceTableOffset();
	if (offsetResourceTable == 0xFFFFFFFF)
		return false;
	if (offsetResourceTable == 0)
		return true;

	if (!readResourceTable(offsetResourceTable))
		return false;

	return true;
}

bool NEResources::loadFromCompressedEXE(const String &fileName) {
	// Based on http://www.cabextract.org.uk/libmspack/doc/szdd_kwaj_format.html

	// TODO: Merge this with with loadFromEXE() so the handling of the compressed
	// EXE's is transparent

	File file;

	if (!file.open(fileName))
		return false;

	// First part of the signature
	if (file.readUint32BE() != MKTAG('S','Z','D','D'))
		return false;

	// Second part of the signature
	if (file.readUint32BE() != 0x88F02733)
		return false;

	// Compression mode must be 'A'
	if (file.readByte() != 'A')
		return false;

	file.readByte(); // file name character change
	uint32 unpackedLength = file.readUint32LE();

	byte *window = new byte[0x1000];
	int pos = 0x1000 - 16;
	memset(window, 0x20, 0x1000); // Initialize to all spaces

	byte *unpackedData = (byte *)malloc(unpackedLength);
	if (!unpackedData)
		error("Failed to allocate uncompressed EXE");

	byte *dataPos = unpackedData;
	byte *endPos = unpackedData + unpackedLength;

	// Apply simple LZSS decompression
	while (dataPos < endPos) {
		byte controlByte = file.readByte();

		if (file.eos())
			break;

		for (byte i = 0; i < 8 && dataPos < endPos; i++) {
			if (controlByte & (1 << i)) {
				*dataPos++ = window[pos++] = file.readByte();
				pos &= 0xFFF;
			} else {
				int matchPos = file.readByte();
				int matchLen = file.readByte();
				matchPos |= (matchLen & 0xF0) << 4;
				matchLen = (matchLen & 0xF) + 3;

				// Clip the length to the remaining size
				matchLen = MIN<int>(matchLen, endPos - dataPos);

				while (matchLen--) {
					*dataPos++ = window[pos++] = window[matchPos++];
					pos &= 0xFFF;
					matchPos &= 0xFFF;
				}
			}

		}
	}

	delete[] window;
	SeekableReadStream *stream = new MemoryReadStream(unpackedData, unpackedLength, DisposeAfterUse::YES);

	return loadFromEXE(stream);
}

uint32 NEResources::getResourceTableOffset() {
	if (!_exe)
		return 0xFFFFFFFF;

	if (!_exe->seek(0))
		return 0xFFFFFFFF;

	//                          'MZ'
	if (_exe->readUint16BE() != 0x4D5A)
		return 0xFFFFFFFF;

	if (!_exe->seek(60))
		return 0xFFFFFFFF;

	uint32 offsetSegmentEXE = _exe->readUint16LE();
	if (!_exe->seek(offsetSegmentEXE))
		return 0xFFFFFFFF;

	//                          'NE'
	if (_exe->readUint16BE() != 0x4E45)
		return 0xFFFFFFFF;

	if (!_exe->seek(offsetSegmentEXE + 36))
		return 0xFFFFFFFF;

	uint32 offsetResourceTable = _exe->readUint16LE();
	if (offsetResourceTable == 0)
		// No resource table
		return 0;

	// Offset relative to the segment _exe header
	offsetResourceTable += offsetSegmentEXE;
	if (!_exe->seek(offsetResourceTable))
		return 0xFFFFFFFF;

	return offsetResourceTable;
}

static const char *s_resTypeNames[] = {
	"", "cursor", "bitmap", "icon", "menu", "dialog", "string",
	"font_dir", "font", "accelerator", "rc_data", "msg_table",
	"group_cursor", "", "group_icon", "", "version", "dlg_include",
	"", "plug_play", "vxd", "ani_cursor", "ani_icon", "html",
	"manifest"
};

bool NEResources::readResourceTable(uint32 offset) {
	if (!_exe)
		return false;

	if (!_exe->seek(offset))
		return false;

	uint32 align = 1 << _exe->readUint16LE();
	uint16 typeID = _exe->readUint16LE();

	while (typeID != 0) {
		// High bit of the type means integer type
		WinResourceID type;
		if (typeID & 0x8000)
			type = typeID & 0x7FFF;
		else
			type = getResourceString(*_exe, offset + typeID);

		uint16 resCount = _exe->readUint16LE();

		_exe->skip(4); // reserved

		for (int i = 0; i < resCount; i++) {
			Resource res;

			// Resource properties
			res.offset = _exe->readUint16LE() * align;
			res.size   = _exe->readUint16LE() * align;
			res.flags  = _exe->readUint16LE();
			uint16 id  = _exe->readUint16LE();
			res.handle = _exe->readUint16LE();
			res.usage  = _exe->readUint16LE();

			res.type = type;

			// High bit means integer type
			if (id & 0x8000)
				res.id = id & 0x7FFF;
			else
				res.id = getResourceString(*_exe, offset + id);

			if (typeID & 0x8000 && ((typeID & 0x7FFF) < ARRAYSIZE(s_resTypeNames)) && s_resTypeNames[typeID & 0x7FFF][0] != 0)
				debug(2, "Found resource %s %s", s_resTypeNames[typeID & 0x7FFF], res.id.toString().c_str());
			else
				debug(2, "Found resource %s %s", type.toString().c_str(), res.id.toString().c_str());

			_resources.push_back(res);
		}

		typeID = _exe->readUint16LE();
	}

	return true;
}

String NEResources::getResourceString(SeekableReadStream &exe, uint32 offset) {
	uint32 curPos = exe.pos();

	if (!exe.seek(offset)) {
		exe.seek(curPos);
		return "";
	}

	uint8 length = exe.readByte();

	String string;
	for (uint16 i = 0; i < length; i++)
		string += (char)exe.readByte();

	exe.seek(curPos);
	return string;
}

const NEResources::Resource *NEResources::findResource(const WinResourceID &type, const WinResourceID &id) const {
	for (List<Resource>::const_iterator it = _resources.begin(); it != _resources.end(); ++it)
		if (it->type == type && it->id == id)
			return &*it;

	return 0;
}

SeekableReadStream *NEResources::getResource(const WinResourceID &type, const WinResourceID &id) {
	const Resource *res = findResource(type, id);

	if (!res)
		return 0;

	_exe->seek(res->offset);
	return _exe->readStream(res->size);
}

const Array<WinResourceID> NEResources::getIDList(const WinResourceID &type) const {
	Array<WinResourceID> idArray;

	for (List<Resource>::const_iterator it = _resources.begin(); it != _resources.end(); ++it)
		if (it->type == type)
			idArray.push_back(it->id);

	return idArray;
}

NEResources::VersionInfo NEResources::getVersionInfo() {
	VersionInfo info;
	Common::ScopedPtr<Common::SeekableReadStream> stream(getResource(kNEVersion, 1));

	if (!stream)
		return info;

	stream->readUint16LE(); // resource size

	// Value size check
	if (stream->readUint16LE() != 0x34)
		return info;

	char versionInfoString[16];
	stream->read(versionInfoString, sizeof(versionInfoString));

	if (memcmp(versionInfoString, "VS_VERSION_INFO", sizeof(versionInfoString) - 1) != 0)
		return info;

	// Signature check
	if (stream->readUint32LE() != 0xFEEF04BD)
		return info;

	stream->readUint32LE(); // struct version

	// The versions are stored a bit weird
	info.fileVersion[1] = stream->readUint16LE();
	info.fileVersion[0] = stream->readUint16LE();
	info.fileVersion[3] = stream->readUint16LE();
	info.fileVersion[2] = stream->readUint16LE();
	info.productVersion[1] = stream->readUint16LE();
	info.productVersion[0] = stream->readUint16LE();
	info.productVersion[3] = stream->readUint16LE();
	info.productVersion[2] = stream->readUint16LE();
	
	info.fileFlagsMask = stream->readUint32LE();
	info.fileFlags = stream->readUint32LE();
	info.fileOS = stream->readUint32LE();
	info.fileType = stream->readUint32LE();
	info.fileSubtype = stream->readUint32LE();
	info.fileDate[0] = stream->readUint32LE();
	info.fileDate[1] = stream->readUint32LE();

	// TODO: Think about reading StringFileInfo and Translation parts

	return info;
}

String NEResources::loadString(uint32 stringID) {
	// This is how the resource ID is calculated
	String string;
	SeekableReadStream *stream = getResource(kNEString, (stringID >> 4) + 1);

	if (!stream)
		return string;

	// Skip over strings we don't care about
	uint32 startString = stringID & ~0xF;

	for (uint32 i = startString; i < stringID; i++)
		stream->skip(stream->readByte());

	byte size = stream->readByte();
	while (size--)
		string += (char)stream->readByte();

	delete stream;
	return string;
}

NEResources::VersionInfo::VersionInfo() {
	fileVersion[0] = fileVersion[1] = fileVersion[2] = fileVersion[3] = 0;
	productVersion[0] = productVersion[1] = productVersion[2] = productVersion[3] = 0;
	fileFlagsMask = 0;
	fileFlags = 0;
	fileOS = 0;
	fileType = 0;
	fileSubtype = 0;
	fileDate[0] = fileDate[1] = 0;
}

bool NEResources::VersionInfo::isValid() const {
	return fileOS != 0;
}

} // End of namespace Common
