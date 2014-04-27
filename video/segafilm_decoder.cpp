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

#include "common/endian.h"
#include "common/scummsys.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"

#include "video/segafilm_decoder.h"
#include "image/codecs/cinepak.h"

namespace Video {

// TODO: Think about moving this to a separate codecs/raw.h file
// For raw video, it seems to always be 24bpp RGB
// We just convert to the current screen format for ease of use
class SegaFilmRawCodec : public Image::Codec {
public:
	SegaFilmRawCodec(uint16 width, uint16 height, byte bitsPerPixel) {
		_surface = new Graphics::Surface();
		_surface->create(width, height, g_system->getScreenFormat());
		_bitsPerPixel = bitsPerPixel;
	}

	~SegaFilmRawCodec() {
		_surface->free();
		delete _surface;
	}

	const Graphics::Surface *decodeFrame(Common::SeekableReadStream &stream) {
		if (_bitsPerPixel != 24) {
			warning("Unhandled %d bpp", _bitsPerPixel);
			return 0;
		}

		if (stream.size() != _surface->w * _surface->h * (_bitsPerPixel >> 3)) {
			warning("Mismatched raw video size");
			return 0;
		}

		for (int32 i = 0; i < _surface->w * _surface->h; i++) {
			byte r = stream.readByte();
			byte g = stream.readByte();
			byte b = stream.readByte();

			if (_surface->format.bytesPerPixel == 2)
				*((uint16 *)_surface->getPixels() + i) = _surface->format.RGBToColor(r, g, b);
			else
				*((uint32 *)_surface->getPixels() + i) = _surface->format.RGBToColor(r, g, b);
		}

		return _surface;
	}

	Graphics::PixelFormat getPixelFormat() const { return _surface->format; }

private:
	Graphics::Surface *_surface;
	byte _bitsPerPixel;
};

struct ChunkTableEntry {
	uint32 offset;
	uint32 size;
	uint32 time;
	uint32 syncBytes;
};

SegaFILMDecoder::SegaFILMDecoder() : _stream(0) {
}

SegaFILMDecoder::~SegaFILMDecoder() {
	close();
}

bool SegaFILMDecoder::loadStream(Common::SeekableReadStream *stream) {
	close();

	_stream = stream;

	// FILM Header
	if (_stream->readUint32BE() != MKTAG('F', 'I', 'L', 'M')) {
		close();
		return false;
	}

	uint32 filmHeaderLength = _stream->readUint32BE();
	uint32 filmVersion = _stream->readUint32BE();
	_stream->readUint32BE(); // Reserved

	// We don't support the SegaCD/Batman and Robin variants
	if (filmVersion == 0x20000) {
		close();
		return false;
	}

	// FDSC Chunk
	if (_stream->readUint32BE() != MKTAG('F', 'D', 'S', 'C')) {
		close();
		return false;
	}

	uint32 fdscChunkSize = _stream->readUint32BE();
	uint32 codecTag = _stream->readUint32BE();
	uint32 height = _stream->readUint32BE();
	uint32 width = _stream->readUint32BE();

	if (codecTag == 0) {
		warning("Unsupported audio-only Sega FILM file");
		close();
		return false;
	}

	byte bitsPerPixel = 24;
	byte audioChannels = 0;
	byte audioSampleSize = 0;
	uint16 audioSampleRate = 0;
	uint audioFlags = 0;
	bool hasSound = false;
	bool planarSound = false;

	if (fdscChunkSize >= 32) {
		bitsPerPixel = _stream->readByte();
		audioChannels = _stream->readByte();
		audioSampleSize = _stream->readByte();
		_stream->readByte(); // Unknown
		audioSampleRate = _stream->readUint16BE();
		_stream->skip(6);

		if (audioChannels == 2)
			audioFlags |= Audio::FLAG_STEREO;
		if (audioSampleSize == 16)
			audioFlags |= Audio::FLAG_16BITS;

		if (fdscChunkSize > 32)
			_stream->skip(fdscChunkSize - 32);

		hasSound = audioSampleSize != 0;
		planarSound = true;
	}

	uint32 tag = _stream->readUint32BE();

	if (tag == MKTAG('A', 'D', 'S', 'C')) {
		// Atari Jaguar audio description
		hasSound = true;
		/* uint32 adscSize = */ _stream->readUint32BE();

		// Just a flag here
		uint32 stereoFlag = stream->readUint32BE();
		if (stereoFlag == 1)
			audioFlags |= Audio::FLAG_STEREO;

		// Some strange rate calculation
		uint32 sampleRate = 830965 / ((_stream->readUint32BE() + 1) << 1);
		uint32 sampleFactor = 0xFFFFFFFF / _stream->readUint32BE();
		audioSampleRate = sampleRate + sampleRate / sampleFactor;

		tag = _stream->readUint32BE();
	}

	uint32 frameCount = 0;
	uint32 timeScale = 0;

	if (tag == MKTAG('C', 'T', 'A', 'B')) {
		// Atari Jaguar chunk table
		/* uint32 ctabSize = */ _stream->readUint32BE();
		timeScale = _stream->readUint32BE();
		uint32 chunkCount = _stream->readUint32BE();

		// Load the chunk table
		Common::Array<ChunkTableEntry> chunks;
		chunks.resize(chunkCount);

		for (uint32 i = 0; i < chunkCount; i++) {
			ChunkTableEntry &chunk = chunks[i];
			chunk.offset = _stream->readUint32BE() + filmHeaderLength + 0x40; // 0x40 = sync bytes
			chunk.size = _stream->readUint32BE();
			chunk.time = _stream->readUint32BE();
			chunk.syncBytes = _stream->readUint32BE();
		}

		// Now aggregate all the sample tables
		for (uint32 i = 0; i < chunkCount; i++) {
			const ChunkTableEntry &chunk = chunks[i];
			stream->seek(chunk.offset);

			tag = _stream->readUint32BE();
			if (tag != MKTAG('S', 'T', 'A', 'B')) {
				warning("Failed to find STAB section in chunk %d", i);
				close();
				return false;
			}

			uint32 stabSize = _stream->readUint32BE();
			_stream->readUint32BE(); // duplicate of time scale
			addSampleTableEntries(chunk.offset + stabSize, frameCount);
		}
	} else if (tag == MKTAG('S', 'T', 'A', 'B')) {
		// Sample table

		// The STAB chunk size changes definitions depending on the version anyway...
		/* uint32 stabSize = */ _stream->readUint32BE();
		timeScale = _stream->readUint32BE();
		addSampleTableEntries(filmHeaderLength, frameCount);
	} else {
		// Unknown
		close();
		return false;
	}

	addTrack(new SegaFILMVideoTrack(width, height, codecTag, bitsPerPixel, frameCount, timeScale));

	// Create the audio stream if audio is present
	if (hasSound)
		addTrack(new SegaFILMAudioTrack(audioSampleRate, audioFlags, planarSound));

	_sampleTablePosition = 0;

	return true;
}

void SegaFILMDecoder::close() {
	VideoDecoder::close();

	_sampleTable.clear();
	delete _stream; _stream = 0;
}

void SegaFILMDecoder::readNextPacket() {
	if (endOfVideoTracks())
		return;

	// Decode until we get a video frame
	for (;;) {
		_stream->seek(_sampleTable[_sampleTablePosition].offset);

		if (_sampleTable[_sampleTablePosition].sampleInfo1 == 0xFFFFFFFF) {
			// Planar audio data. All left channel first and then left in stereo.
			SegaFILMAudioTrack *audioTrack = (SegaFILMAudioTrack *)getTrack(1);
			assert(audioTrack);
			audioTrack->queueAudio(_stream, _sampleTable[_sampleTablePosition].length);
			_sampleTablePosition++;
		} else {
			// We have a video frame!
			((SegaFILMVideoTrack *)getTrack(0))->decodeFrame(_stream->readStream(_sampleTable[_sampleTablePosition].length), _sampleTable[_sampleTablePosition].sampleInfo2);
			_sampleTablePosition++;
			return;
		}
	}

	// This should be impossible to get to
	error("Could not find Sega FILM frame %d", getCurFrame());
}

void SegaFILMDecoder::addSampleTableEntries(uint32 offset, uint32 &frameCount) {
	uint32 sampleCount = _stream->readUint32BE();

	for (uint32 i = 0; i < sampleCount; i++) {
		SampleTableEntry sample;
		sample.offset = _stream->readUint32BE() + offset;
		sample.length = _stream->readUint32BE();
		sample.sampleInfo1 = _stream->readUint32BE();
		sample.sampleInfo2 = _stream->readUint32BE();

		// Calculate the frame count based on the number of video samples.
		// 0xFFFFFFFF represents an audio frame.
		if (sample.sampleInfo1 != 0xFFFFFFFF)
			frameCount++;

		_sampleTable.push_back(sample);
	}
}

SegaFILMDecoder::SegaFILMVideoTrack::SegaFILMVideoTrack(uint32 width, uint32 height, uint32 codecTag, byte bitsPerPixel, uint32 frameCount, uint32 timeScale) {
	_width = width;
	_height = height;
	_frameCount = frameCount;
	_timeScale = timeScale;
	_curFrame = -1;
	_nextFrameStartTime = 0;

	// Create the Cinepak decoder, if we're using it
	if (codecTag == MKTAG('c', 'v', 'i', 'd'))
		_codec = new Image::CinepakDecoder();
	else if (codecTag == MKTAG('r', 'a', 'w', ' '))
		_codec = new SegaFilmRawCodec(_width, _height, bitsPerPixel);
	else if (codecTag != 0)
		error("Unknown Sega FILM codec tag '%s'", tag2str(codecTag));
}

SegaFILMDecoder::SegaFILMVideoTrack::~SegaFILMVideoTrack() {
	delete _codec;
}

Graphics::PixelFormat SegaFILMDecoder::SegaFILMVideoTrack::getPixelFormat() const {
	return _codec->getPixelFormat();
}

void SegaFILMDecoder::SegaFILMVideoTrack::decodeFrame(Common::SeekableReadStream *stream, uint32 duration) {
	_surface = _codec->decodeFrame(*stream);
	_curFrame++;
	_nextFrameStartTime += duration; // Add the frame's duration to the next frame start
}

SegaFILMDecoder::SegaFILMAudioTrack::SegaFILMAudioTrack(uint audioFrequency, uint audioFlags, bool usePlanar) {
	_audioFlags = audioFlags;
	_audioStream = Audio::makeQueuingAudioStream(audioFrequency, audioFlags & Audio::FLAG_STEREO);
	_usePlanar = usePlanar;
}

SegaFILMDecoder::SegaFILMAudioTrack::~SegaFILMAudioTrack() {
	delete _audioStream;
}

void SegaFILMDecoder::SegaFILMAudioTrack::queueAudio(Common::SeekableReadStream *stream, uint32 length) {
	// TODO: CRI ADX ADPCM is possible here too
	byte *audioBuffer = (byte *)malloc(length);

	if (_usePlanar) {
		// TODO: Maybe move this to a new class?
		if (_audioFlags & Audio::FLAG_16BITS) {
			if (_audioFlags & Audio::FLAG_STEREO) {
				for (byte i = 0; i < 2; i++)
					for (uint16 j = 0; j < length / 4; j++)
						WRITE_BE_UINT16(audioBuffer + j * 4 + i * 2, stream->readUint16BE());
			} else {
				for (uint16 i = 0; i < length / 2; i++)
					WRITE_BE_UINT16(audioBuffer + i * 2, stream->readUint16BE());
			}
		} else {
			if (_audioFlags & Audio::FLAG_STEREO) {
				for (byte i = 0; i < 2; i++)
					for (uint16 j = 0; j < length / 2; j++)
						audioBuffer[j * 2 + i] = stream->readByte();
			} else {
				stream->read(audioBuffer, length);
			}
		}
	} else {
		stream->read(audioBuffer, length);
	}

	// Now the audio is loaded, so let's queue it
	_audioStream->queueBuffer(audioBuffer, length, DisposeAfterUse::YES, _audioFlags);
}

Audio::AudioStream *SegaFILMDecoder::SegaFILMAudioTrack::getAudioStream() const {
	return _audioStream;
}

} // End of namespace Video
