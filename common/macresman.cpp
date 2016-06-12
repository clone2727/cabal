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

#include "common/scummsys.h"
#include "common/debug.h"
#include "common/util.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/macresman.h"
#include "common/md5.h"
#include "common/ptr.h"
#include "common/substream.h"
#include "common/textconsole.h"

#ifdef MACOSX
#include "common/config-manager.h"
#endif

namespace Common {

#define MBI_INFOHDR 128
#define MBI_ZERO1 0
#define MBI_NAMELEN 1
#define MBI_ZERO2 74
#define MBI_ZERO3 82
#define MBI_DFLEN 83
#define MBI_RFLEN 87
#define MAXNAMELEN 63

MacResManager::MacResManager() : _stream(0), _mode(kResForkNone), _resForkOffset(-1),
		_resForkSize(0), _dataOffset(0), _dataLength(0), _mapOffset(0), _mapLength(0) {
}

MacResManager::~MacResManager() {
	close();
}

void MacResManager::close() {
	_resForkOffset = -1;
	_mode = kResForkNone;
	_resTypeMap.clear();
	delete _stream; _stream = 0;
}

bool MacResManager::hasDataFork() const {
	return !_baseFileName.empty();
}

bool MacResManager::hasResFork() const {
	return !_baseFileName.empty() && _mode != kResForkNone;
}

uint32 MacResManager::getResForkDataSize() const {
	if (!hasResFork())
		return 0;

	_stream->seek(_resForkOffset + 4);
	return _stream->readUint32BE();
}

String MacResManager::computeResForkMD5AsString(uint32 length) const {
	if (!hasResFork())
		return String();

	_stream->seek(_resForkOffset);
	uint32 dataOffset = _stream->readUint32BE() + _resForkOffset;
	/* uint32 mapOffset = */ _stream->readUint32BE();
	uint32 dataLength = _stream->readUint32BE();


	SeekableSubReadStream resForkStream(_stream, dataOffset, dataOffset + dataLength);
	return computeStreamMD5AsString(resForkStream, MIN<uint32>(length, _resForkSize));
}

bool MacResManager::open(const String &fileName) {
	close();

#ifdef MACOSX
	{
		// Check the actual fork on a Mac computer
		String fullPath = ConfMan.get("path") + "/" + fileName + "/..namedfork/rsrc";
		FSNode resFsNode = FSNode(fullPath);
		if (resFsNode.exists()) {
			SeekableReadStream *macResForkRawStream = resFsNode.createReadStream();

			if (macResForkRawStream && loadFromRawFork(*macResForkRawStream)) {
				_baseFileName = fileName;
				return true;
			}

			delete macResForkRawStream;
		}
	}
#endif

	File *file = new File();

	// Prefer standalone files first, starting with raw forks
	if (file->open(fileName + ".rsrc") && loadFromRawFork(*file)) {
		_baseFileName = fileName;
		return true;
	}
	file->close();

	// Then try for AppleDouble using Apple's naming
	if (file->open(constructAppleDoubleName(fileName)) && loadFromAppleDouble(*file)) {
		_baseFileName = fileName;
		return true;
	}
	file->close();

	// Check .bin for MacBinary next
	if (file->open(fileName + ".bin") && loadFromMacBinary(*file)) {
		_baseFileName = fileName;
		return true;
	}
	file->close();

	// As a last resort, see if just the data fork exists
	if (file->open(fileName)) {
		_baseFileName = fileName;

		// FIXME: Is this really needed?
		if (isMacBinary(*file)) {
			file->seek(0);
			if (loadFromMacBinary(*file))
				return true;
		}

		file->seek(0);
		_stream = file;
		return true;
	}

	delete file;

	// The file doesn't exist
	return false;
}

bool MacResManager::open(const FSNode &path, const String &fileName) {
	close();

#ifdef MACOSX
	{
		// Check the actual fork on a Mac computer
		String fullPath = path.getPath() + "/" + fileName + "/..namedfork/rsrc";
		FSNode resFsNode = FSNode(fullPath);
		if (resFsNode.exists()) {
			SeekableReadStream *macResForkRawStream = resFsNode.createReadStream();

			if (macResForkRawStream && loadFromRawFork(*macResForkRawStream)) {
				_baseFileName = fileName;
				return true;
			}

			delete macResForkRawStream;
		}
	}
#endif

	// Prefer standalone files first, starting with raw forks
	FSNode fsNode = path.getChild(fileName + ".rsrc");
	if (fsNode.exists() && !fsNode.isDirectory()) {
		SeekableReadStream *stream = fsNode.createReadStream();
		if (loadFromRawFork(*stream)) {
			_baseFileName = fileName;
			return true;
		}
		delete stream;
	}

	// Then try for AppleDouble using Apple's naming
	fsNode = path.getChild(constructAppleDoubleName(fileName));
	if (fsNode.exists() && !fsNode.isDirectory()) {
		SeekableReadStream *stream = fsNode.createReadStream();
		if (loadFromAppleDouble(*stream)) {
			_baseFileName = fileName;
			return true;
		}
		delete stream;
	}

	// Check .bin for MacBinary next
	fsNode = path.getChild(fileName + ".bin");
	if (fsNode.exists() && !fsNode.isDirectory()) {
		SeekableReadStream *stream = fsNode.createReadStream();
		if (loadFromMacBinary(*stream)) {
			_baseFileName = fileName;
			return true;
		}
		delete stream;
	}

	// As a last resort, see if just the data fork exists
	fsNode = path.getChild(fileName);
	if (fsNode.exists() && !fsNode.isDirectory()) {
		SeekableReadStream *stream = fsNode.createReadStream();
		_baseFileName = fileName;

		// FIXME: Is this really needed?
		if (isMacBinary(*stream)) {
			stream->seek(0);
			if (loadFromMacBinary(*stream))
				return true;
		}

		stream->seek(0);
		_stream = stream;
		return true;
	}

	// The file doesn't exist
	return false;
}

bool MacResManager::exists(const String &fileName) {
	// Try the file name by itself
	if (File::exists(fileName))
		return true;

	// Try the .rsrc extension
	if (File::exists(fileName + ".rsrc"))
		return true;

	// Check if we have a MacBinary file
	File tempFile;
	if (tempFile.open(fileName + ".bin") && isMacBinary(tempFile))
		return true;

	// Check if we have an AppleDouble file
	if (tempFile.open(constructAppleDoubleName(fileName))) {
		uint32 tag = tempFile.readUint32BE();
		if (!tempFile.eos() && tag == 0x00051607)
			return true;
	}

	return false;
}

bool MacResManager::loadFromAppleDouble(SeekableReadStream &stream) {
	uint32 tag = stream.readUint32BE();
	if (stream.eos() || tag != 0x00051607) // tag
		return false;

	stream.skip(20); // version + home file system

	uint16 entryCount = stream.readUint16BE();
	if (stream.eos())
		return false;

	for (uint16 i = 0; i < entryCount; i++) {
		uint32 id = stream.readUint32BE();
		uint32 offset = stream.readUint32BE();
		uint32 length = stream.readUint32BE();
		if (stream.eos())
			return false;

		if (id == 2) {
			// Found the resource fork!
			_resForkOffset = offset;
			_mode = kResForkAppleDouble;
			_resForkSize = length;
			return load(stream);
		}
	}

	return false;
}

bool MacResManager::isMacBinary(SeekableReadStream &stream) {
	byte infoHeader[MBI_INFOHDR];
	uint32 size = stream.read(infoHeader, MBI_INFOHDR);
	if (size != MBI_INFOHDR)
		return false;

	// Check that the header is correct
	if (infoHeader[MBI_ZERO1] == 0 && infoHeader[MBI_ZERO2] == 0 &&
		infoHeader[MBI_ZERO3] == 0 && infoHeader[MBI_NAMELEN] <= MAXNAMELEN) {

		// Pull out fork lengths
		uint32 dataSize = READ_BE_UINT32(infoHeader + MBI_DFLEN);
		uint32 rsrcSize = READ_BE_UINT32(infoHeader + MBI_RFLEN);

		uint32 dataSizePad = (((dataSize + 127) >> 7) << 7);
		uint32 rsrcSizePad = (((rsrcSize + 127) >> 7) << 7);

		// If the header + data size + rsrc size equal the stream size, it should
		// be MacBinary.
		if (MBI_INFOHDR + dataSizePad + rsrcSizePad == (uint32)stream.size())
			return true;
	}

	// Not detected as MacBinary
	return false;
}

bool MacResManager::loadFromMacBinary(SeekableReadStream &stream) {
	byte infoHeader[MBI_INFOHDR];
	uint32 size = stream.read(infoHeader, MBI_INFOHDR);
	if (size != MBI_INFOHDR)
		return false;

	// Maybe we have MacBinary?
	if (infoHeader[MBI_ZERO1] == 0 && infoHeader[MBI_ZERO2] == 0 &&
		infoHeader[MBI_ZERO3] == 0 && infoHeader[MBI_NAMELEN] <= MAXNAMELEN) {

		// Pull out fork lengths
		uint32 dataSize = READ_BE_UINT32(infoHeader + MBI_DFLEN);
		uint32 rsrcSize = READ_BE_UINT32(infoHeader + MBI_RFLEN);

		uint32 dataSizePad = (((dataSize + 127) >> 7) << 7);
		uint32 rsrcSizePad = (((rsrcSize + 127) >> 7) << 7);

		// Length check
		if (MBI_INFOHDR + dataSizePad + rsrcSizePad == (uint32)stream.size()) {
			_resForkOffset = MBI_INFOHDR + dataSizePad;
			_resForkSize = rsrcSize;
		}
	}

	if (_resForkOffset < 0)
		return false;

	_mode = kResForkMacBinary;
	return load(stream);
}

bool MacResManager::loadFromRawFork(SeekableReadStream &stream) {
	_mode = kResForkRaw;
	_resForkOffset = 0;
	_resForkSize = stream.size();
	return load(stream);
}

bool MacResManager::load(SeekableReadStream &stream) {
	if (_mode == kResForkNone)
		return false;

	stream.seek(_resForkOffset);

	_dataOffset = stream.readUint32BE() + _resForkOffset;
	_mapOffset = stream.readUint32BE() + _resForkOffset;
	_dataLength = stream.readUint32BE();
	_mapLength = stream.readUint32BE();

	// do sanity check
	if (stream.eos() || _dataOffset >= (uint32)stream.size() || _mapOffset >= (uint32)stream.size() ||
			_dataLength + _mapLength  > (uint32)stream.size()) {
		_resForkOffset = -1;
		_mode = kResForkNone;
		return false;
	}

	debug(7, "Resource fork header: data %d [%d] map %d [%d]",
		_dataOffset, _dataLength, _mapOffset, _mapLength);

	_stream = &stream;

	readMap();
	return true;
}

SeekableReadStream *MacResManager::getDataFork() {
	if (!_stream)
		return NULL;

	if (_mode == kResForkMacBinary) {
		_stream->seek(MBI_DFLEN);
		uint32 dataSize = _stream->readUint32BE();
		return new SeekableSubReadStream(_stream, MBI_INFOHDR, MBI_INFOHDR + dataSize);
	}

	File *file = new File();
	if (file->open(_baseFileName))
		return file;
	delete file;

	return NULL;
}

MacResIDArray MacResManager::getResIDArray(uint32 typeID) const {
	MacResIDArray res;

	ResourceTypeMap::const_iterator typeIt = _resTypeMap.find(typeID);
	if (typeIt == _resTypeMap.end())
		return res;

	const ResourceMap &resMap = typeIt->_value;
	res.reserve(resMap.size());

	for (ResourceMap::const_iterator it = resMap.begin(); it != resMap.end(); it++)
		res.push_back(it->_key);

	return res;
}

MacResTagArray MacResManager::getResTagArray() const {
	MacResTagArray tagArray;

	tagArray.reserve(_resTypeMap.size());

	for (ResourceTypeMap::const_iterator it = _resTypeMap.begin(); it != _resTypeMap.end(); it++)
		tagArray.push_back(it->_key);

	return tagArray;
}

String MacResManager::getResName(uint32 typeID, uint16 resID) const {
	ResourceTypeMap::const_iterator typeIt = _resTypeMap.find(typeID);
	if (typeIt == _resTypeMap.end())
		return "";

	const ResourceMap &resMap = typeIt->_value;
	ResourceMap::const_iterator it = resMap.find(resID);
	if (it == resMap.end())
		return "";

	return it->_value.name;
}

SeekableReadStream *MacResManager::getResource(uint32 typeID, uint16 resID) {
	ResourceTypeMap::const_iterator typeIt = _resTypeMap.find(typeID);
	if (typeIt == _resTypeMap.end())
		return 0;

	const ResourceMap &resMap = typeIt->_value;
	ResourceMap::const_iterator it = resMap.find(resID);
	if (it == resMap.end())
		return 0;

	const Resource &resource = it->_value;
	_stream->seek(_dataOffset + resource.dataOffset);
	uint32 len = _stream->readUint32BE();

	// Ignore resources with 0 length
	if (len == 0)
		return 0;

	return _stream->readStream(len);
}

SeekableReadStream *MacResManager::getResource(const String &fileName) {
	if (fileName.empty())
		return 0;

	for (ResourceTypeMap::const_iterator typeIt = _resTypeMap.begin(); typeIt != _resTypeMap.end(); typeIt++) {
		const ResourceMap &resMap = typeIt->_value;

		for (ResourceMap::const_iterator it = resMap.begin(); it != resMap.end(); it++) {
			const Resource &resource = it->_value;

			if (fileName.equalsIgnoreCase(resource.name)) {
				_stream->seek(_dataOffset + resource.dataOffset);
				uint32 len = _stream->readUint32BE();

				// Ignore resources with 0 length
				if (len == 0)
					return 0;

				return _stream->readStream(len);
			}
		}
	}

	return 0;
}

SeekableReadStream *MacResManager::getResource(uint32 typeID, const String &fileName) {
	if (fileName.empty())
		return 0;

	ResourceTypeMap::const_iterator typeIt = _resTypeMap.find(typeID);
	if (typeIt == _resTypeMap.end())
		return 0;

	const ResourceMap &resMap = typeIt->_value;

	for (ResourceMap::const_iterator it = resMap.begin(); it != resMap.end(); it++) {
		const Resource &resource = it->_value;

		if (fileName.equalsIgnoreCase(resource.name)) {
			_stream->seek(_dataOffset + resource.dataOffset);
			uint32 len = _stream->readUint32BE();

			// Ignore resources with 0 length
			if (len == 0)
				return 0;

			return _stream->readStream(len);
		}
	}

	return 0;
}

void MacResManager::readMap() {
	_stream->seek(_mapOffset + 22);

	/*uint16 resAttr = */ _stream->readUint16BE();
	uint16 typeOffset = _stream->readUint16BE();
	uint16 nameOffset = _stream->readUint16BE();
	uint32 typeCount = _stream->readUint16BE() + 1;

	_stream->seek(_mapOffset + typeOffset + 2);

	// Make some temporary maps to help with parsing
	typedef HashMap<uint32, uint32> ItemCountMap;
	typedef HashMap<uint32, uint16> OffsetMap;

	ItemCountMap itemCountMap;
	OffsetMap offsetMap;

	for (uint32 i = 0; i < typeCount; i++) {
		uint32 id = _stream->readUint32BE();
		uint32 itemCount = _stream->readUint16BE() + 1;
		uint16 offset = _stream->readUint16BE();

		itemCountMap[id] = itemCount;
		offsetMap[id] = offset;

		debug(8, "Resource Type <%s>: items = %d, offset = %d (0x%x)", tag2str(id), itemCount, offset, offset);
	}

	for (ItemCountMap::const_iterator it = itemCountMap.begin(); it != itemCountMap.end(); it++) {
		uint32 itemCount = it->_value;
		ResourceMap resMap;

		_stream->seek(_mapOffset + typeOffset + offsetMap[it->_key]);

		// Read in the basic item info
		for (uint32 i = 0; i < itemCount; i++) {
			uint16 id = _stream->readUint16BE();

			Resource res;
			uint16 resNameOffset = _stream->readUint16BE();
			res.dataOffset = _stream->readUint32BE();
			_stream->readUint32BE();

			res.attr = res.dataOffset >> 24;
			res.dataOffset &= 0xFFFFFF;

			// Pull out the name, if available
			if (resNameOffset != 0xFFFF) {
				// Store the position so we can get back after getting the name
				uint32 lastPos = _stream->pos();

				// Pull the string out of the name table
				_stream->seek(_mapOffset + nameOffset + resNameOffset);
				byte nameSize = _stream->readByte();
				ScopedArray<char> nameBuf(new char[nameSize]);
				_stream->read(nameBuf.get(), nameSize);
				res.name = String(nameBuf.get(), nameSize);

				// Seek back to the next entry
				_stream->seek(lastPos);
			}

			resMap[id] = res;
		}

		_resTypeMap[it->_key] = resMap;
	}
}

String MacResManager::constructAppleDoubleName(const String &path) {
	// Try to find the last slash in the string
	const char *lastPathComponent = strrchr(path.c_str(), '/');

	if (lastPathComponent) {
		// Skip the actual slash so it gets copied into the right spot
		lastPathComponent++;
		return String(path.c_str(), lastPathComponent) + "._" + lastPathComponent;
	}

	// Can just insert "._" before the beginning of the string
	return "._" + path;
}

} // End of namespace Common
