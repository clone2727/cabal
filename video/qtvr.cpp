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
#include "common/textconsole.h"
#include "video/qtvr.h"

namespace Video {

QTVRDecoder::QTVRDecoder() {
	_version = 0;
	_qtvrTrack = _panoTrack = 0;

	initQTVRParseTable();
}

bool QTVRDecoder::loadFile(const Common::String &filename) {
	close();

	if (!QuickTimeDecoder::loadFile(filename))
		return false;

	return loadNodeData(); 
}

bool QTVRDecoder::loadStream(Common::SeekableReadStream *stream) {
	close();

	if (!QuickTimeDecoder::loadStream(stream))
		return false;

	return loadNodeData(); 
}

void QTVRDecoder::close() {
	_version = 0;
	_qtvrTrack = _panoTrack = 0;
	QuickTimeDecoder::close();
}

bool QTVRDecoder::loadNodeData() {
	// If version is 0, we didn't detect the QTVR version, and this isn't a QTVR file
	if (_version == 0)
		return false;

	// We had to have found the panoramic track
	// (There's no plan to support object VR files currently)
	if (!_panoTrack)
		return false;

	// v2 requires the QTVR track
	if (_version == 2 && !_qtvrTrack)
		return false;

	return true;
}

void QTVRDecoder::initQTVRParseTable() {
#define DECLARE_PARSER(func, tag) \
	_parseTable.push_back(new AtomParseEntry(new Common::Functor1Mem<Atom, int, QTVRDecoder>(this, &QTVRDecoder::func), tag))

	// QTVR v1 atoms
	DECLARE_PARSER(readDefault, MKTAG('S', 'T', 'p', 'n'));
	DECLARE_PARSER(readPINF, MKTAG('p', 'I', 'n', 'f'));

	// QTVR v2 atoms
	DECLARE_PARSER(readVRSG, MKTAG('v', 'r', 's', 'g'));
	DECLARE_PARSER(readVRSC, MKTAG('v', 'r', 's', 'c'));
	DECLARE_PARSER(readIMPN, MKTAG('i', 'm', 'p', 'n'));
	DECLARE_PARSER(readNLOC, MKTAG('n', 'l', 'o', 'c'));

#undef DECLARE_PARSER
}

Common::QuickTimeParser::SampleDesc *QTVRDecoder::readSampleDesc(Common::QuickTimeParser::Track *track, uint32 format, uint32 descSize) {	
	if (track->codecType == CODEC_TYPE_PANO_V1) {
		// Panoramic sample description in v1
		_panoDesc.majorVersion = _fd->readUint16BE();
		_panoDesc.minorVersion = _fd->readUint16BE();
		_panoDesc.sceneTrackID = _fd->readUint32BE();
		_panoDesc.lowResSceneTrackID = _fd->readUint32BE();
		_fd->skip(4 * 6); // reserved
		_panoDesc.hotspotTrackID = _fd->readUint32BE();
		_fd->skip(4 * 9); // reserved
		_panoDesc.hPanStart = Common::Rational(_fd->readSint32BE(), 0x10000);
		_panoDesc.hPanEnd = Common::Rational(_fd->readSint32BE(), 0x10000);
		_panoDesc.vPanTop = Common::Rational(_fd->readSint32BE(), 0x10000);
		_panoDesc.vPanBottom = Common::Rational(_fd->readSint32BE(), 0x10000);
		_panoDesc.minZoom = Common::Rational(_fd->readSint32BE(), 0x10000);
		_panoDesc.maxZoom = Common::Rational(_fd->readSint32BE(), 0x10000);
		_panoDesc.sceneSizeX = _fd->readUint32BE();
		_panoDesc.sceneSizeY = _fd->readUint32BE();
		_panoDesc.frameCount = _fd->readUint32BE();
		/* uint16 reserved5 = */ _fd->readUint16BE();
		_panoDesc.sceneFrameCountX = _fd->readUint16BE();
		_panoDesc.sceneFrameCountY = _fd->readUint16BE();
		_panoDesc.sceneColorDepth = _fd->readUint16BE();
		_panoDesc.hotspotSizeX = _fd->readUint32BE();
		_panoDesc.hotspotSizeY = _fd->readUint32BE();
		/* uint16 reserved6 = */ _fd->readUint16BE();
		_panoDesc.hotspotFrameCountX = _fd->readUint16BE();
		_panoDesc.hotspotFrameCountY = _fd->readUint16BE();
		_panoDesc.hotspotColorDepth = _fd->readUint16BE();

		// Mark this as the pano track
		_panoTrack = Common::QuickTimeParser::_tracks.back();

		// We don't need a sample desc, so return 0
		return 0;
	} else if (track->codecType == CODEC_TYPE_QTVR) {
		// The data here is an atom container
		Atom a = { 0, 0, 0, 0, 0 };
		a.size = descSize;
		readAtomContainer(a);

		// Mark this as the QTVR track
		_qtvrTrack = Common::QuickTimeParser::_tracks.back();

		// Don't need a sample desc here either
		return 0;
	} else if (track->codecType == CODEC_TYPE_PANO_V2) {
		// Mark this as the pano track
		_panoTrack = Common::QuickTimeParser::_tracks.back();

		// No sample description
		return 0;
	}

	// Pass it on up
	return QuickTimeDecoder::readSampleDesc(track, format, descSize);
}

int QTVRDecoder::readPINF(Atom atom) {
	// This marks a v1 QTVR file
	_version = 1;
	debug(0, "Detected QTVR v1 file");

	// Read the name
	byte nameSize = _fd->readByte();
	char name[32];
	_fd->read(name, 31);
	name[nameSize] = 0;
	_panoMediaInfo.name = name;
	debug(0, "Scene Name: '%s'", name);

	_panoMediaInfo.defaultNodeID = _fd->readUint32BE();
	_panoMediaInfo.defaultZoom = Common::Rational(_fd->readUint32BE(), 0x10000);
	debug(0, "Default Node ID: %d", _panoMediaInfo.defaultNodeID);
	debug(0, "Default Zoom: %f", _panoMediaInfo.defaultZoom.toDouble());

	/* uint32 reserved = */ _fd->readUint32BE();
	/* uint16 pad = */ _fd->readUint16BE();

	uint16 idCount = _fd->readUint16BE();
	debug(0, "ID to Time: %d entries", idCount);

	for (uint16 i = 0; i < idCount; i++) {
		IDTableEntry entry;
		entry.nodeID = _fd->readUint32BE();
		entry.time = _fd->readUint32BE();
		_panoMediaInfo.idToTime.push_back(entry);
		debug(1, "Node %d, Time = %d", entry.nodeID, entry.time);
	}

	return 0;
}

int QTVRDecoder::readVRSG(Atom atom) {
	// We currently don't do anything with the string, except for printing it

	_fd->readUint16BE(); // string usage, unused
	uint16 stringLength = _fd->readUint16BE();

	// TODO: a 'vrse' atom may describe this string's encoding
	// Just assume ASCII

	Common::String description;
	for (uint16 i = 0; i < stringLength; i++)
		description += (char)_fd->readByte();

	debug(0, "VR String: '%s'", description.c_str());
	return 0;
}

int QTVRDecoder::readVRSC(Atom atom) {
	// This is a v2 QTVR file
	_version = 2;
	debug(0, "Detected QTVR v2 file");

	_worldHeader.majorVersion = _fd->readUint16BE();
	_worldHeader.minorVersion = _fd->readUint16BE();
	_worldHeader.nameAtomID = _fd->readUint32BE();
	_worldHeader.defaultNodeID = _fd->readUint32BE();
	_worldHeader.vrWorldFlags = _fd->readUint32BE();
	/* uint32 reserved1 = */ _fd->readUint32BE();
	/* uint32 reserved2 = */ _fd->readUint32BE();

	debug(0, "VR Header: v%d.%d, name atom = %d, default node = %d, flags = %08x", _worldHeader.majorVersion, _worldHeader.minorVersion, _worldHeader.nameAtomID, _worldHeader.defaultNodeID, _worldHeader.vrWorldFlags);
	return 0;
}

int QTVRDecoder::readIMPN(Atom atom) {
	PanoImaging panoImaging;
	panoImaging.majorVersion = _fd->readUint16BE();
	panoImaging.minorVersion = _fd->readUint16BE();
	panoImaging.imagingMode = _fd->readUint32BE();
	panoImaging.imagingValidFlags = _fd->readUint32BE();
	panoImaging.correction = _fd->readUint32BE();
	panoImaging.quality = _fd->readUint32BE();
	panoImaging.directDraw = _fd->readUint32BE();

	// Skip reserved properties
	for (int i = 0; i < 6; i++)
		_fd->readUint32BE();

	/* uint32 reserved1 = */ _fd->readUint32BE();
	/* uint32 reserved2 = */ _fd->readUint32BE();

	debug(0, "Pano Imaging: v%d.%d, mode = %x, flags = %08x, correction = %d, quality = %d, direct draw = %d", panoImaging.majorVersion, panoImaging.minorVersion, panoImaging.imagingMode, panoImaging.imagingValidFlags, panoImaging.correction, panoImaging.quality, panoImaging.directDraw);

	_panoImaging.push_back(panoImaging);
	return 0;
}

int QTVRDecoder::readNLOC(Atom atom) {
	NodeLocation loc;
	loc.majorVersion = _fd->readUint16BE();
	loc.minorVersion = _fd->readUint16BE();
	loc.nodeType = _fd->readUint32BE();
	loc.locationFlags = _fd->readUint32BE();
	loc.locationData = _fd->readUint32BE();
	/* uint32 reserved1 = */ _fd->readUint32BE();
	/* uint32 reserved2 = */ _fd->readUint32BE();

	debug(0, "Node Location: v%d.%d, type = '%s', flags = %08x, data = %d", loc.majorVersion, loc.minorVersion, tag2str(loc.nodeType), loc.locationFlags, loc.locationData);

	_nodeLocations.push_back(loc);
	return 0;
}

} // End of namespace Video

