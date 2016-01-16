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

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "backends/audiodev/win32/win32_pcspk.h"

#include "common/debug.h"
#include "common/scummsys.h"
#include "common/textconsole.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace Audio {

/**
 * A PC speaker device using InpOut to speak directly to the hardware
 */
class Win32PCSpeaker : public HardwarePCSpeaker {
public:
	Win32PCSpeaker();
	~Win32PCSpeaker();

	// AudioDevice API
	bool init();
	void reset();

	// PCSpeakerDevice API
	void startOutput(int freq);
	void startOutputTicks(int ticks);
	void stopOutput();

private:
	typedef void (__stdcall *Out32Proc)(short, short);
	typedef short (__stdcall *Inp32Proc)(short);
	typedef BOOL (__stdcall *IsInpOutDriverOpenProc)();

	Out32Proc _outProc;
	Inp32Proc _inProc;
	IsInpOutDriverOpenProc _openProc;
	HINSTANCE _dll;

	void closeLibrary();
};

Win32PCSpeaker::Win32PCSpeaker() : _dll(0), _outProc(0), _inProc(0), _openProc(0) {
}

Win32PCSpeaker::~Win32PCSpeaker() {
	stop();
	stopOutput();
	closeLibrary();
}

bool Win32PCSpeaker::init() {
	closeLibrary();

#ifdef SCUMM_64BITS
	_dll = LoadLibrary("InpOutx64.dll");
	if (!_dll) {
		warning("Failed to open InpOutx64.dll");
		return false;
	}
#else
	_dll = LoadLibrary("InpOut32.dll");
	if (!_dll) {
		warning("Failed to open InpOut32.dll");
		return false;
	}
#endif

	_outProc = (Out32Proc)GetProcAddress(_dll, "Out32");
	_inProc = (Inp32Proc)GetProcAddress(_dll, "Inp32");
	_openProc = (IsInpOutDriverOpenProc)GetProcAddress(_dll, "IsInpOutDriverOpen");

	if (!_outProc || !_inProc || !_openProc) {
		warning("Failed to retrieve InpOut32 procs");
		closeLibrary();
		return false;
	}

	if (!_openProc()) {
		warning("Failed to initialize InpOut");
		closeLibrary();
		return false;
	}

	// Set up the speaker
	_outProc(0x43, 0xB6);

	// Ensure no note is playing
	stopOutput();
	return true;
}

void Win32PCSpeaker::reset() {
	// Ensure no note is playing
	stopOutput();
}

void Win32PCSpeaker::startOutput(int freq) {
	if (freq == 0) {
		stopOutput();
		return;
	}

	startOutputTicks(kDeviceFreq / freq);
}

void Win32PCSpeaker::startOutputTicks(int ticks) {
	if (!_dll)
		return;

	if (ticks == 0) {
		stopOutput();
		return;
	}

	// Write the three registers
	_outProc(0x42, ticks & 0xff);
	_outProc(0x42, (ticks >> 8) & 0xff);	
	_outProc(0x61, _inProc(0x61) | 0x03);
}

void Win32PCSpeaker::stopOutput() {
	if (!_dll)
		return;

	// Disable playback
	_outProc(0x61, _inProc(0x61) & 0xFC);
}

void Win32PCSpeaker::closeLibrary() {
	_outProc = 0;
	_inProc = 0;
	_openProc = 0;

	if (_dll) {
		FreeLibrary(_dll);
		_dll = 0;
	}
}

PCSpeakerDevice *createWin32PCSpeaker() {
	return new Win32PCSpeaker();
}

} // End of namespace Audio

