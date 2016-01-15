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

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/kd.h>
#include <linux/input.h>
#include <sys/ioctl.h>

#include "audio/audiodev/pcspk.h"
#include "backends/audiodev/linux/linux_pcspk.h"
#include "common/debug.h"

namespace Audio {

/**
 * A PC speaker device using the Linux native ioctl interface
 */
class LinuxPCSpeaker : public HardwarePCSpeaker {
public:
	LinuxPCSpeaker();
	~LinuxPCSpeaker();

	// AudioDevice API
	bool init();
	void reset();

	// PCSpeakerDevice API
	void startOutput(int freq);
	void startOutputTicks(int ticks);
	void stopOutput();

private:
	int _fd;
	bool _useIoctl;
	void closeFd();
	bool setFrequency(int freq);
};

LinuxPCSpeaker::LinuxPCSpeaker() : _fd(-1), _useIoctl(false) {
}

LinuxPCSpeaker::~LinuxPCSpeaker() {
	stop();
	stopOutput();
	closeFd();
}

bool LinuxPCSpeaker::init() {
	closeFd();

	// First try for the preferred method, input
	_useIoctl = false;
	_fd = ::open("/dev/input/by-path/platform-pcspkr-event-spkr", O_WRONLY);
	if (_fd < 0) {
		// Fallback on the /dev/console method
		debug(1, "Failed to open /dev/input/by-path/platform-pcspkr-event-spkr: %s", strerror(errno));
		_fd = ::open("/dev/console", O_WRONLY);
		if (_fd < 0) {
			debug(1, "Failed to open /dev/console: %s", strerror(errno));
			return false;
		}

		_useIoctl = true;
	}

	// Ensure no note is playing
	stopOutput();
	return true;
}

void LinuxPCSpeaker::reset() {
	// Ensure no note is playing
	stopOutput();
}

void LinuxPCSpeaker::startOutput(int freq) {
	setFrequency(freq);
}

void LinuxPCSpeaker::startOutputTicks(int ticks) {
	if (ticks == 0)
		setFrequency(0);
	else
		setFrequency(kDeviceFreq / ticks);
}

void LinuxPCSpeaker::stopOutput() {
	setFrequency(0);
}

void LinuxPCSpeaker::closeFd() {
	if (_fd >= 0) {
		::close(_fd);
		_fd = -1;
	}
}

bool LinuxPCSpeaker::setFrequency(int freq) {
	if (_fd < 0)
		return false;

	// Handle the ioctl method
	if (_useIoctl)
		return ioctl(_fd, KIOCSOUND, freq) < 0;

	// Input method
	input_event event;
	event.type = EV_SND;
	event.code = SND_TONE;
	event.value = freq;
	return write(_fd, &event, sizeof(event)) == sizeof(event);
}

PCSpeakerDevice *createLinuxPCSpeaker() {
	return new LinuxPCSpeaker();
}

} // End of namespace Audio

