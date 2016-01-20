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

#ifndef VIDEO_MPEGPS_DECODER_H
#define VIDEO_MPEGPS_DECODER_H

#include "common/hashmap.h"
#include "graphics/surface.h"
#include "video/video_decoder.h"

namespace Audio {
class PacketizedAudioStream;
}

namespace Common {
class SeekableReadStream;
}

namespace Graphics {
struct PixelFormat;
}

namespace Image {
class MPEGDecoder;
}

namespace Video {

/**
 * Decoder for MPEG Program Stream videos.
 * Video decoder used in engines:
 *  - zvision
 */
class MPEGPSDecoder : public VideoDecoder {
public:
	MPEGPSDecoder();
	virtual ~MPEGPSDecoder();

	bool loadStream(Common::SeekableReadStream *stream);
	void close();

protected:
	void readNextPacket();

private:
	// Base class for handling MPEG streams
	class MPEGStream {
	public:
		MPEGStream(int startCode) : _startCode(startCode), _lastPTS(0), _lastDTS(0), _nextStartOffset(0), _endOfStream(false) {}
		virtual ~MPEGStream() {}

		enum StreamType {
			kStreamTypeVideo,
			kStreamTypeAudio
		};

		bool sendPacket(Common::SeekableReadStream *packet, uint32 offset, uint32 pts, uint32 dts);
		virtual StreamType getStreamType() const = 0;
		uint32 getLastPTS() const { return _lastPTS; }
		uint32 getLastDTS() const { return _lastDTS; }
		uint32 getNextStartOffset() const { return _nextStartOffset; }
		int getStartCode() const { return _startCode; }
		void setEndOfStream() { _endOfStream = true; }
		bool isEndOfStream() const { return _endOfStream; }

	protected:
		virtual bool decodePacket(Common::SeekableReadStream *packet) = 0;

	private:
		int _startCode;
		uint32 _lastPTS;
		uint32 _lastDTS;
		uint32 _nextStartOffset;
		bool _endOfStream;
	};

	// Base class for all MPEG video streams
	class MPEGVideoStream : public VideoTrack, public MPEGStream {
	public:
		MPEGVideoStream(int startCode) : MPEGStream(startCode) {}

		/**
		 * Get the timestamp of the next frame
		 */
		virtual Audio::Timestamp getNextFrameStartTimestamp() const = 0;

		// MPEGStream API
		StreamType getStreamType() const { return kStreamTypeVideo; }

		// VideoTrack API
		uint32 getNextFrameStartTime() const { return getNextFrameStartTimestamp().msecs(); }
	};

	// Base class for all MPEG audio streams
	class MPEGAudioStream : public AudioTrack, public MPEGStream {
	public:
		MPEGAudioStream(int startCode) : MPEGStream(startCode) {}

		// MPEGStream API
		StreamType getStreamType() const { return kStreamTypeAudio; }
	};

	// An MPEG 1/2 video track
	class MPEGVideoTrack : public MPEGVideoStream {
	public:
		MPEGVideoTrack(int startCode, Common::SeekableReadStream *firstPacket, const Graphics::PixelFormat &format);
		~MPEGVideoTrack();

		// VideoTrack API
		bool endOfTrack() const { return isEndOfStream(); }
		uint16 getWidth() const;
		uint16 getHeight() const;
		Graphics::PixelFormat getPixelFormat() const;
		int getCurFrame() const { return _curFrame; }
		const Graphics::Surface *decodeNextFrame();

		// MPEGVideoStream API
		Audio::Timestamp getNextFrameStartTimestamp() const { return _nextFrameStartTime; }

	protected:
		bool decodePacket(Common::SeekableReadStream *packet);

	private:
		int _curFrame;
		Audio::Timestamp _nextFrameStartTime;
		Graphics::Surface *_surface;

		void findDimensions(Common::SeekableReadStream *firstPacket, const Graphics::PixelFormat &format);

#ifdef USE_MPEG2
		Image::MPEGDecoder *_mpegDecoder;
#endif
	};

#ifdef USE_MAD
	// An MPEG audio track
	class MPEGAudioTrack : public MPEGAudioStream {
	public:
		MPEGAudioTrack(int startCode, Common::SeekableReadStream &firstPacket);
		~MPEGAudioTrack();

	protected:
		bool decodePacket(Common::SeekableReadStream *packet);
		Audio::AudioStream *getAudioStream() const;

	private:
		Audio::PacketizedAudioStream *_audStream;
	};
#endif

#ifdef USE_A52
	class AC3AudioTrack : public MPEGAudioStream {
	public:
		AC3AudioTrack(Common::SeekableReadStream &firstPacket);
		~AC3AudioTrack();

	protected:
		bool decodePacket(Common::SeekableReadStream *packet);
		Audio::AudioStream *getAudioStream() const;

	private:
		Audio::PacketizedAudioStream *_audStream;
	};
#endif

	// The different types of private streams we can detect at the moment
	enum PrivateStreamType {
		kPrivateStreamUnknown,
		kPrivateStreamAC3,
		kPrivateStreamDTS,
		kPrivateStreamDVDPCM,
		kPrivateStreamPS2Audio
	};

	PrivateStreamType detectPrivateStreamType(Common::SeekableReadStream *packet);

	bool addFirstVideoTrack();

	int readNextPacketHeader(int32 &startCode, uint32 &pts, uint32 &dts);
	int findNextStartCode(uint32 &size);
	uint32 readPTS(int c);
	void handleNextPacket(MPEGStream &stream);
	void addNewStream(int startCode, uint32 packetSize);

	void parseProgramStreamMap(int length);
	byte _psmESType[256];

	// A map from stream types to stream handlers
	typedef Common::HashMap<int, MPEGStream *> StreamMap;
	StreamMap _streamMap;
	Common::Array<MPEGVideoStream *> _videoStreams;
	Common::Array<MPEGAudioStream *> _audioStreams;

	Common::SeekableReadStream *_stream;
};

} // End of namespace Video

#endif
