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

#ifndef VIDEO_QTVR_H
#define VIDEO_QTVR_H

#include "common/rect.h"
#include "common/hashmap.h"

#include "video/qt_decoder.h"

namespace Video {

/**
 * A player for QTVR nodes
 */
class QTVRDecoder : private QuickTimeDecoder {
public:
	QTVRDecoder();
	~QTVRDecoder() { close(); }

	bool loadFile(const Common::String &filename);
	bool loadStream(Common::SeekableReadStream *stream);
	bool isLoaded() const { return _version != 0; }
	void close();

protected:
	Common::QuickTimeParser::SampleDesc *readSampleDesc(Common::QuickTimeParser::Track *track, uint32 format, uint32 descSize);

private:
	bool loadNodeData();

	// Custom QTVR v1 atom types
	int readPINF(Atom atom);
	int readPHDR(Atom atom);
	int readPHOT(Atom atom);
	int readSTRT(Atom atom);
	int readPLNK(Atom atom);
	int readPNAV(Atom atom);

	// Custom QTVR v2 atom types
	int readVRSG(Atom atom);
	int readVRSC(Atom atom);
	int readIMPN(Atom atom);
	int readNLOC(Atom atom);

	void initQTVRParseTable();

	void readQTVRTracks();

	// Version of QTVR
	uint _version;

	Common::QuickTimeParser::Track *_qtvrTrack, *_panoTrack;

	// v1.0

	struct IDTableEntry {
		uint32 nodeID;
		uint32 time;
	};

	struct PanoMediaInfo {
		Common::String name;
		uint32 defaultNodeID;
		Common::Rational defaultZoom;
		Common::Array<IDTableEntry> idToTime;
	} _panoMediaInfo;

	struct PanoramaDescription {
		uint16 majorVersion;
		uint16 minorVersion;
		uint32 sceneTrackID;
		uint32 lowResSceneTrackID;
		uint32 hotspotTrackID;
		Common::Rational hPanStart;
		Common::Rational hPanEnd;
		Common::Rational vPanTop;
		Common::Rational vPanBottom;
		Common::Rational minZoom;
		Common::Rational maxZoom;
		uint32 sceneSizeX;
		uint32 sceneSizeY;
		uint32 frameCount;
		uint16 sceneFrameCountX;
		uint16 sceneFrameCountY;
		uint16 sceneColorDepth;
		uint32 hotspotSizeX;
		uint32 hotspotSizeY;
		uint16 hotspotFrameCountX;
		uint16 hotspotFrameCountY;
		uint16 hotspotColorDepth;
	} _panoDesc;

	struct Hotspot {
		uint16 hotspotID;
		uint32 type;
		uint32 typeData;
		Common::Rational viewHPan;
		Common::Rational viewVPan;
		Common::Rational viewZoom;
		Common::Rect hotspotRect;
		uint32 mouseOverCursorID;
		uint32 mouseDownCursorID;
		uint32 mouseUpCursorID;
		uint32 nameStrOffset;
		uint32 commentStrOffset;
	};

	struct Link {
		uint16 linkID;
		uint32 toNodeID;
		Common::Rational toHPan;
		Common::Rational toVPan;
		Common::Rational toZoom;
		uint32 nameStrOffset;
		uint32 commentStrOffset;
	};

	struct Navigation {
		uint16 objID;
		Common::Rational navgHPan;
		Common::Rational navgVPan;
		Common::Rational navgZoom;
		Common::Rect zoomRect;
		uint32 nameStrOffset;
		uint32 commentStrOffset;
	};

	struct Node {
		uint32 nodeID;
		Common::Rational defHPan;
		Common::Rational defVPan;
		Common::Rational defZoom;
		Common::Rational minHPan;
		Common::Rational minVPan;
		Common::Rational minZoom;
		Common::Rational maxHPan;
		Common::Rational maxVPan;
		Common::Rational maxZoom;
		uint32 nameStrOffset;
		uint32 commentStrOffset;

		Common::Array<Hotspot> hotspots;
		Common::HashMap<uint32, Common::String> stringTable;
		Common::Array<Link> links;
		Common::Array<Navigation> navigations;
	};

	Common::Array<Node> _nodes;

	// v2.0

	struct QTVRWorldHeader {
		uint16 majorVersion;
		uint16 minorVersion;
		uint32 nameAtomID;
		uint32 defaultNodeID;
		uint32 vrWorldFlags;
	} _worldHeader;

	enum PanoImagingFlags {
		kQTVRValidCorrection = 1 << 0,
		kQTVRValidQuality = 1 << 1,
		kQTVRValidDirectDraw = 1 << 2,
		kQTVRValidFirstExtraProperty = 1 << 3	
	};

	struct PanoImaging {
		uint16 majorVersion;
		uint16 minorVersion;
		uint32 imagingMode;
		uint32 imagingValidFlags;
		uint32 correction;
		uint32 quality;
		uint32 directDraw;
	};

	Common::Array<PanoImaging> _panoImaging;

	struct NodeLocation {
		uint32 nodeID;
		uint16 majorVersion;
		uint16 minorVersion;
		uint32 nodeType;
		uint32 locationFlags;
		uint32 locationData;
	};

	Common::Array<NodeLocation> _nodeLocations;
};

} // End of namespace Video

#endif

