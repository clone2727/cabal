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

#include "audio/decoders/vorbis.h"

#ifdef USE_VORBIS

#include "common/mutex.h"
#include "common/ptr.h"
#include "common/queue.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "common/util.h"

#include "audio/audiostream.h"

#ifdef USE_TREMOR
#ifdef USE_TREMOLO
#include <tremolo/ivorbisfile.h>
#include <tremolo/ivorbiscodec.h>
#else
#include <tremor/ivorbisfile.h>
#include <tremor/ivorbiscodec.h>
#endif
#else
#define OV_EXCLUDE_STATIC_CALLBACKS
#include <vorbis/vorbisfile.h>
#include <vorbis/codec.h>
#endif


namespace Audio {

// These are wrapper functions to allow using a SeekableReadStream object to
// provide data to the OggVorbis_File object.

static size_t read_stream_wrap(void *ptr, size_t size, size_t nmemb, void *datasource) {
	Common::SeekableReadStream *stream = (Common::SeekableReadStream *)datasource;

	uint32 result = stream->read(ptr, size * nmemb);

	return result / size;
}

static int seek_stream_wrap(void *datasource, ogg_int64_t offset, int whence) {
	Common::SeekableReadStream *stream = (Common::SeekableReadStream *)datasource;
	stream->seek((int32)offset, whence);
	return stream->pos();
}

static int close_stream_wrap(void *datasource) {
	// Do nothing -- we leave it up to the VorbisStream to free memory as appropriate.
	return 0;
}

static long tell_stream_wrap(void *datasource) {
	Common::SeekableReadStream *stream = (Common::SeekableReadStream *)datasource;
	return stream->pos();
}

static ov_callbacks g_stream_wrap = {
	read_stream_wrap, seek_stream_wrap, close_stream_wrap, tell_stream_wrap
};

class VorbisStream : public SeekableAudioStream {
protected:
	Common::DisposablePtr<Common::SeekableReadStream> _inStream;

	uint _channels;
	int _rate;

	Common::Timestamp _length;

	OggVorbis_File _ovFile;

	int16 _buffer[4096];
	const int16 *_bufferEnd;
	const int16 *_pos;

public:
	// startTime / duration are in milliseconds
	VorbisStream(Common::SeekableReadStream *inStream, DisposeAfterUse::Flag dispose);
	~VorbisStream();

	int readBuffer(int16 *buffer, const int numSamples);

	bool endOfData() const		{ return _pos >= _bufferEnd; }
	uint getChannels() const { return _channels; }
	int getRate() const			{ return _rate; }

	bool seek(const Common::Timestamp &where);
	Common::Timestamp getLength() const { return _length; }
protected:
	bool refill();
};

VorbisStream::VorbisStream(Common::SeekableReadStream *inStream, DisposeAfterUse::Flag dispose) :
	_inStream(inStream, dispose),
	_length(0, 1000),
	_bufferEnd(ARRAYEND(_buffer)) {

	int res = ov_open_callbacks(inStream, &_ovFile, NULL, 0, g_stream_wrap);
	if (res < 0) {
		warning("Could not create Vorbis stream (%d)", res);
		_pos = _bufferEnd;
		return;
	}

	// Read in initial data
	if (!refill())
		return;

	// Setup some header information
	_channels = ov_info(&_ovFile, -1)->channels;
	_rate = ov_info(&_ovFile, -1)->rate;

#ifdef USE_TREMOR
	_length = Common::Timestamp(ov_time_total(&_ovFile, -1), getRate());
#else
	_length = Common::Timestamp(uint32(ov_time_total(&_ovFile, -1) * 1000.0), getRate());
#endif
}

VorbisStream::~VorbisStream() {
	ov_clear(&_ovFile);
}

int VorbisStream::readBuffer(int16 *buffer, const int numSamples) {
	int samples = 0;
	while (samples < numSamples && _pos < _bufferEnd) {
		const int len = MIN(numSamples - samples, (int)(_bufferEnd - _pos));
		memcpy(buffer, _pos, len * 2);
		buffer += len;
		_pos += len;
		samples += len;
		if (_pos >= _bufferEnd) {
			if (!refill())
				break;
		}
	}
	return samples;
}

bool VorbisStream::seek(const Common::Timestamp &where) {
	// Vorbisfile uses the sample pair number, thus we always use 1 for the channels parameter
	// of the convertTimeToStreamPos helper.
	int res = ov_pcm_seek(&_ovFile, convertTimeToStreamPos(where, getRate(), 1).totalNumberOfFrames());
	if (res) {
		warning("Error seeking in Vorbis stream (%d)", res);
		_pos = _bufferEnd;
		return false;
	}

	return refill();
}

bool VorbisStream::refill() {
	// Read the samples
	uint len_left = sizeof(_buffer);
	char *read_pos = (char *)_buffer;

	while (len_left > 0) {
		long result;

#ifdef USE_TREMOR
		// Tremor ov_read() always returns data as signed 16 bit interleaved PCM
		// in host byte order. As such, it does not take arguments to request
		// specific signedness, byte order or bit depth as in Vorbisfile.
		result = ov_read(&_ovFile, read_pos, len_left,
						NULL);
#else
#ifdef SCUMM_BIG_ENDIAN
		result = ov_read(&_ovFile, read_pos, len_left,
						1,
						2,	// 16 bit
						1,	// signed
						NULL);
#else
		result = ov_read(&_ovFile, read_pos, len_left,
						0,
						2,	// 16 bit
						1,	// signed
						NULL);
#endif
#endif
		if (result == OV_HOLE) {
			// Possibly recoverable, just warn about it
			warning("Corrupted data in Vorbis file");
		} else if (result == 0) {
			//warning("End of file while reading from Vorbis file");
			//_pos = _bufferEnd;
			//return false;
			break;
		} else if (result < 0) {
			warning("Error reading from Vorbis stream (%d)", int(result));
			_pos = _bufferEnd;
			// Don't delete it yet, that causes problems in
			// the CD player emulation code.
			return false;
		} else {
			len_left -= result;
			read_pos += result;
		}
	}

	_pos = _buffer;
	_bufferEnd = (int16 *)read_pos;

	return true;
}

class PacketizedVorbisStream : public PacketizedAudioStream {
public:
	PacketizedVorbisStream();
	~PacketizedVorbisStream();

	bool parseExtraData(Common::SeekableReadStream &stream);
	bool parseExtraData(Common::SeekableReadStream &packet1, Common::SeekableReadStream &packet2, Common::SeekableReadStream &packet3);

	// AudioStream API
	uint getChannels() const { return _vorbisInfo.channels; }
	int getRate() const { return _vorbisInfo.rate; }
	int readBuffer(int16 *buffer, const int numSamples);
	bool endOfData() const;
	bool endOfStream() const;

	// PacketizedAudioStream API
	void queuePacket(Common::SeekableReadStream *packet);
	void finish();

private:
	// Vorbis decode state
	vorbis_info _vorbisInfo;
	vorbis_dsp_state _dspState;
	vorbis_block _block;
	vorbis_comment _comment;
	ogg_packet _packet;
	bool _init;

	Common::Mutex _mutex;
	Common::Queue<Common::SeekableReadStream *> _queue;
	bool _finished;
	bool _hasData;
};

PacketizedVorbisStream::PacketizedVorbisStream() {
	_finished = false;
	_init = false;
	_hasData = false;
	vorbis_info_init(&_vorbisInfo);
	vorbis_comment_init(&_comment);
}

PacketizedVorbisStream::~PacketizedVorbisStream() {
	if (_init) {
		vorbis_block_clear(&_block);
		vorbis_dsp_clear(&_dspState);
	}

	vorbis_info_clear(&_vorbisInfo);
	vorbis_comment_clear(&_comment);

	// Remove anything from the queue
	while (!_queue.empty())
		delete _queue.pop();
}

bool PacketizedVorbisStream::parseExtraData(Common::SeekableReadStream &stream) {
	if (stream.size() < 3)
		return false;

	byte initialBytes[3];
	stream.read(initialBytes, sizeof(initialBytes));

	int headerSizes[3];
	Common::ScopedArray<byte> headers[3];

	if (stream.size() >= 6 && READ_BE_UINT16(initialBytes) == 30) {
		stream.seek(0);

		for (int i = 0; i < 3; i++) {
			headerSizes[i] = stream.readUint16BE();

			if (headerSizes[i] + stream.pos() > stream.size()) {
				warning("Vorbis header size invalid");
				return false;
			}

			headers[i].reset(new byte[headerSizes[i]]);
			stream.read(headers[i].get(), headerSizes[i]);
		}
	} else if (initialBytes[0] == 2 && stream.size() < 0x7FFFFE00) {
		stream.seek(1);
		uint32 offset = 1;

		for (int i = 0; i < 2; i++) {
			headerSizes[i] = 0;

			while (stream.pos() < stream.size()) {
				byte length = stream.readByte();
				headerSizes[i] += length;
				offset++;

				if (length != 0xFF)
					break;
			}

			if (offset >= (uint32)stream.size()) {
				warning("Vorbis header sizes damaged");
				return false;
			}
		}

		headerSizes[2] = stream.size() - (headerSizes[0] + headerSizes[1] + offset);
		stream.seek(offset);

		for (int i = 0; i < 3; i++) {
			headers[i].reset(new byte[headerSizes[i]]);
			stream.read(headers[i].get(), headerSizes[i]);
		}
	} else {
		warning("Invalid vorbis initial header length: %d", initialBytes[0]);
		return false;
	}

	for (int i = 0; i < 3; i++) {
		_packet.b_o_s = (i == 0);
		_packet.bytes = headerSizes[i];
		_packet.packet = headers[i].get();

		if (vorbis_synthesis_headerin(&_vorbisInfo, &_comment, &_packet) < 0) {
			warning("Vorbis header %d is damaged", i);
			return false;
		}
	}

	// Begin decode
	vorbis_synthesis_init(&_dspState, &_vorbisInfo);
	vorbis_block_init(&_dspState, &_block);
	_init = true;

	return true;
}

bool PacketizedVorbisStream::parseExtraData(Common::SeekableReadStream &packet1, Common::SeekableReadStream &packet2, Common::SeekableReadStream &packet3) {
	int headerSizes[3];
	Common::ScopedArray<byte> headers[3];

#define READ_WHOLE_STREAM(x) \
	do { \
		Common::SeekableReadStream &packet = packet##x; \
		headerSizes[x - 1] = packet.size(); \
		headers[x - 1].reset(new byte[headerSizes[x - 1]]); \
		packet.read(headers[x - 1].get(), headerSizes[x - 1]); \
	} while (0)

	READ_WHOLE_STREAM(1);
	READ_WHOLE_STREAM(2);
	READ_WHOLE_STREAM(3);

#undef READ_WHOLE_STREAM

	for (int i = 0; i < 3; i++) {
		_packet.b_o_s = (i == 0);
		_packet.bytes = headerSizes[i];
		_packet.packet = headers[i].get();

		if (vorbis_synthesis_headerin(&_vorbisInfo, &_comment, &_packet) < 0) {
			warning("Vorbis header %d is damaged", i);
			return false;
		}
	}

	// Begin decode
	vorbis_synthesis_init(&_dspState, &_vorbisInfo);
	vorbis_block_init(&_dspState, &_block);
	_init = true;

	return true;
}

int PacketizedVorbisStream::readBuffer(int16 *buffer, const int numSamples) {
	int samples = 0;
	while (samples < numSamples) {
#ifdef USE_TREMOR
		ogg_int32_t **pcm;
#else
		float **pcm;
#endif
		int decSamples = vorbis_synthesis_pcmout(&_dspState, &pcm);
		if (decSamples <= 0) {
			// No more samples
			Common::StackLock lock(_mutex);
			_hasData = false;

			// If the queue is empty, we can do nothing else
			if (_queue.empty())
				return samples;

			// Feed the next packet into the beast
			Common::ScopedPtr<Common::SeekableReadStream> stream(_queue.pop());
			Common::ScopedArray<byte> data(new byte[stream->size()]);
			stream->read(data.get(), stream->size());

			// Synthesize!
			_packet.packet = data.get();
			_packet.bytes = stream->size();
			if (vorbis_synthesis(&_block, &_packet) == 0) {
				vorbis_synthesis_blockin(&_dspState, &_block);
				_hasData = true;
			} else {
				warning("Failed to synthesize from vorbis packet");
			}

			// Retry pcmout
			continue;
		}

		// See how many samples we can decode
		decSamples = MIN<int>((numSamples - samples) / getChannels(), decSamples);

#ifdef USE_TREMOR
		for (int i = 0; i < decSamples; i++)
			for (uint j = 0; j < getChannels(); j++)
				buffer[samples++] = (int16)(pcm[j][i] / 32768);
#else
		for (int i = 0; i < decSamples; i++)
			for (uint j = 0; j < getChannels(); j++)
				buffer[samples++] = CLIP<int>(floor(pcm[j][i] * 32767.0f + 0.5), -32768, 32767);
#endif

		vorbis_synthesis_read(&_dspState, decSamples);
	}

	return samples;
}

bool PacketizedVorbisStream::endOfData() const {
	Common::StackLock lock(_mutex);
	return !_hasData && _queue.empty();
}

bool PacketizedVorbisStream::endOfStream() const {
	Common::StackLock lock(_mutex);
	return _finished && endOfData();
}

void PacketizedVorbisStream::queuePacket(Common::SeekableReadStream *packet) {
	Common::StackLock lock(_mutex);
	assert(!_finished);
	_queue.push(packet);
}

void PacketizedVorbisStream::finish() {
	Common::StackLock lock(_mutex);
	_finished = true;
}

SeekableAudioStream *makeVorbisStream(
	Common::SeekableReadStream *stream,
	DisposeAfterUse::Flag disposeAfterUse) {
	SeekableAudioStream *s = new VorbisStream(stream, disposeAfterUse);
	if (s && s->endOfData()) {
		delete s;
		return 0;
	} else {
		return s;
	}
}

PacketizedAudioStream *makePacketizedVorbisStream(Common::SeekableReadStream &extraData) {
	Common::ScopedPtr<PacketizedVorbisStream> stream(new PacketizedVorbisStream());
	if (!stream->parseExtraData(extraData))
		return 0;

	return stream.release();
}

PacketizedAudioStream *makePacketizedVorbisStream(Common::SeekableReadStream &packet1, Common::SeekableReadStream &packet2, Common::SeekableReadStream &packet3) {
	Common::ScopedPtr<PacketizedVorbisStream> stream(new PacketizedVorbisStream());
	if (!stream->parseExtraData(packet1, packet2, packet3))
		return 0;

	return stream.release();
}

} // End of namespace Audio

#endif // #ifdef USE_VORBIS
