#include <cxxtest/TestSuite.h>

#include "audio/audiostream.h"

#include "helper.h"

class AudioStreamTestSuite : public CxxTest::TestSuite
{
public:
	void test_convertTimeToStreamPos() {
		const Common::Timestamp a = Audio::convertTimeToStreamPos(Common::Timestamp(500, 1000), 11025, 2);
		// The last bit has to be 0 in any case for a stereo stream.
		TS_ASSERT_EQUALS(a.totalNumberOfFrames() & 1, 0);

		// TODO: This test is rather hacky... actually converTimeToStreamPos might also return 11026
		// instead of 11024 and it would still be a valid sample position for ~500ms.
		TS_ASSERT_EQUALS(a.totalNumberOfFrames(), 11024);

		const Common::Timestamp b = Audio::convertTimeToStreamPos(Common::Timestamp(500, 1000), 11025, 1);
		TS_ASSERT_EQUALS(b.totalNumberOfFrames(), 500 * 11025 / 1000);

		// Test Audio CD positioning

		// for 44kHz and stereo
		const Common::Timestamp c = Audio::convertTimeToStreamPos(Common::Timestamp(0, 50, 75), 44100, 2);
		TS_ASSERT_EQUALS(c.totalNumberOfFrames(), 50 * 44100 * 2 / 75);

		// for 11kHz and mono
		const Common::Timestamp d = Audio::convertTimeToStreamPos(Common::Timestamp(0, 50, 75), 11025, 1);
		TS_ASSERT_EQUALS(d.totalNumberOfFrames(), 50 * 11025 / 75);

		// Some misc test
		const Common::Timestamp e = Audio::convertTimeToStreamPos(Common::Timestamp(1, 1, 4), 11025, 1);
		TS_ASSERT_EQUALS(e.totalNumberOfFrames(), 5 * 11025 / 4);
	}

private:
	void testLoopingAudioStreamFixedIter(const int sampleRate, const uint channels) {
		const int secondLength = sampleRate * channels;

		int16 *sine = 0;
		Audio::SeekableAudioStream *s = createSineStream<int16>(sampleRate, 1, &sine, false, channels);
		Audio::LoopingAudioStream *loop = new Audio::LoopingAudioStream(s, 7);

		int16 *buffer = new int16[secondLength * 3];

		// Check parameters
		TS_ASSERT_EQUALS(loop->getChannels(), channels);
		TS_ASSERT_EQUALS(loop->getRate(), sampleRate);
		TS_ASSERT_EQUALS(loop->endOfData(), false);
		TS_ASSERT_EQUALS(loop->getCompleteIterations(), (uint)0);

		// Read one second
		TS_ASSERT_EQUALS(loop->readBuffer(buffer, secondLength), secondLength);
		TS_ASSERT_EQUALS(memcmp(buffer, sine, secondLength * sizeof(int16)), 0);

		TS_ASSERT_EQUALS(loop->getCompleteIterations(), (uint)1);
		TS_ASSERT_EQUALS(loop->endOfData(), false);

		// Read two seconds
		TS_ASSERT_EQUALS(loop->readBuffer(buffer, secondLength * 2), secondLength * 2);
		TS_ASSERT_EQUALS(memcmp(buffer, sine, secondLength * sizeof(int16)), 0);
		TS_ASSERT_EQUALS(memcmp(buffer + secondLength, sine, secondLength * sizeof(int16)), 0);

		TS_ASSERT_EQUALS(loop->getCompleteIterations(), (uint)3);
		TS_ASSERT_EQUALS(loop->endOfData(), false);

		// Read three seconds
		TS_ASSERT_EQUALS(loop->readBuffer(buffer, secondLength * 3), secondLength * 3);
		TS_ASSERT_EQUALS(memcmp(buffer, sine, secondLength * sizeof(int16)), 0);
		TS_ASSERT_EQUALS(memcmp(buffer + secondLength, sine, secondLength * sizeof(int16)), 0);
		TS_ASSERT_EQUALS(memcmp(buffer + secondLength * 2, sine, secondLength * sizeof(int16)), 0);

		TS_ASSERT_EQUALS(loop->getCompleteIterations(), (uint)6);
		TS_ASSERT_EQUALS(loop->endOfData(), false);

		// Read the last second in two parts
		const int firstStep = secondLength / 2;
		const int secondStep = secondLength - firstStep;

		TS_ASSERT_EQUALS(loop->readBuffer(buffer, firstStep), firstStep);
		TS_ASSERT_EQUALS(memcmp(buffer, sine, firstStep * sizeof(int16)), 0);

		TS_ASSERT_EQUALS(loop->getCompleteIterations(), (uint)6);
		TS_ASSERT_EQUALS(loop->endOfData(), false);

		TS_ASSERT_EQUALS(loop->readBuffer(buffer, secondLength), secondStep);
		TS_ASSERT_EQUALS(memcmp(buffer, sine + firstStep, secondStep * sizeof(int16)), 0);

		TS_ASSERT_EQUALS(loop->getCompleteIterations(), (uint)7);
		TS_ASSERT_EQUALS(loop->endOfData(), true);

		// Try to read beyond the end of the stream
		TS_ASSERT_EQUALS(loop->readBuffer(buffer, secondLength), 0);
		TS_ASSERT_EQUALS(loop->getCompleteIterations(), (uint)7);
		TS_ASSERT_EQUALS(loop->endOfData(), true);

		delete[] buffer;
		delete loop;
		delete[] sine;
	}

public:
	void test_looping_audio_stream_mono_11025_fixed_iter() {
		testLoopingAudioStreamFixedIter(11025, 1);
	}

	void test_looping_audio_stream_mono_22050_fixed_iter() {
		testLoopingAudioStreamFixedIter(22050, 1);
	}

	void test_looping_audio_stream_stereo_11025_fixed_iter() {
		testLoopingAudioStreamFixedIter(11025, 2);
	}

	void test_looping_audio_stream_stereo_22050_fixed_iter() {
		testLoopingAudioStreamFixedIter(22050, 2);
	}

private:
	void testSubLoopingAudioStreamFixedIter(const int sampleRate, const uint channels, const int time, const int loopEndTime) {
		const int secondLength = sampleRate * channels;

		const Common::Timestamp loopStart(500, 1000), loopEnd(loopEndTime * 1000, 1000);

		const int32 loopStartPos = Audio::convertTimeToStreamPos(loopStart, sampleRate, channels).totalNumberOfFrames();
		const int32 loopEndPos = Audio::convertTimeToStreamPos(loopEnd, sampleRate, channels).totalNumberOfFrames();

		const int32 loopIteration = loopEndPos - loopStartPos;

		int16 *sine = 0;
		Audio::SeekableAudioStream *s = createSineStream<int16>(sampleRate, time, &sine, false, channels);
		Audio::SubLoopingAudioStream *loop = new Audio::SubLoopingAudioStream(s, 5, loopStart, loopEnd);

		const int32 bufferLen = MAX<int32>(loopIteration * 3, loopEndPos);
		int16 *buffer = new int16[bufferLen];

		// Check parameters
		TS_ASSERT_EQUALS(loop->getChannels(), channels);
		TS_ASSERT_EQUALS(loop->getRate(), sampleRate);
		TS_ASSERT_EQUALS(loop->endOfData(), false);

		// Read the non-looped part + one iteration
		TS_ASSERT_EQUALS(loop->readBuffer(buffer, loopEndPos), loopEndPos);
		TS_ASSERT_EQUALS(memcmp(buffer, sine, loopEndPos * sizeof(int16)), 0);
		TS_ASSERT_EQUALS(loop->endOfData(), false);

		// We should have one full iteration now

		// Read another loop iteration
		TS_ASSERT_EQUALS(loop->readBuffer(buffer, loopIteration), loopIteration);
		TS_ASSERT_EQUALS(memcmp(buffer, sine + loopStartPos, loopIteration * sizeof(int16)), 0);
		TS_ASSERT_EQUALS(loop->endOfData(), false);

		// We should have two full iterations now

		// Read three loop iterations at once
		TS_ASSERT_EQUALS(loop->readBuffer(buffer, loopIteration * 3), loopIteration * 3);
		TS_ASSERT_EQUALS(memcmp(buffer + loopIteration * 0, sine + loopStartPos, loopIteration * sizeof(int16)), 0);
		TS_ASSERT_EQUALS(memcmp(buffer + loopIteration * 1, sine + loopStartPos, loopIteration * sizeof(int16)), 0);
		TS_ASSERT_EQUALS(memcmp(buffer + loopIteration * 2, sine + loopStartPos, loopIteration * sizeof(int16)), 0);
		TS_ASSERT_EQUALS(loop->endOfData(), true);

		// We should have five full iterations now, thus the stream should be done

		// Try to read beyond the end of the stream (note this only applies, till we define that SubLoopingAudioStream should
		// stop playing after the looped area).
		TS_ASSERT_EQUALS(loop->readBuffer(buffer, secondLength), 0);
		TS_ASSERT_EQUALS(loop->endOfData(), true);

		delete[] buffer;
		delete loop;
		delete[] sine;
	}

public:
	void test_sub_looping_audio_stream_mono_11025_mid_fixed_iter() {
		testSubLoopingAudioStreamFixedIter(11025, 1, 2, 1);
	}

	void test_sub_looping_audio_stream_mono_22050_mid_fixed_iter() {
		testSubLoopingAudioStreamFixedIter(22050, 1, 2, 1);
	}

	void test_sub_looping_audio_stream_stereo_11025_mid_fixed_iter() {
		testSubLoopingAudioStreamFixedIter(11025, 2, 2, 1);
	}

	void test_sub_looping_audio_stream_stereo_22050_mid_fixed_iter() {
		testSubLoopingAudioStreamFixedIter(22050, 2, 2, 1);
	}

	void test_sub_looping_audio_stream_mono_11025_end_fixed_iter() {
		testSubLoopingAudioStreamFixedIter(11025, 1, 2, 2);
	}

	void test_sub_looping_audio_stream_mono_22050_end_fixed_iter() {
		testSubLoopingAudioStreamFixedIter(22050, 1, 2, 2);
	}

	void test_sub_looping_audio_stream_stereo_11025_end_fixed_iter() {
		testSubLoopingAudioStreamFixedIter(11025, 2, 2, 2);
	}

	void test_sub_looping_audio_stream_stereo_22050_end_fixed_iter() {
		testSubLoopingAudioStreamFixedIter(22050, 2, 2, 2);
	}
};
