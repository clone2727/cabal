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

 // Partially based on djtubig-malicex's OPL3 synth driver (LGPL)
 // https://bitbucket.org/djtubig-malicex/opl3-synth-driver/src

#include "common/scummsys.h"

#include "common/debug.h"
#include "common/str.h"
#include "common/textconsole.h"
#include "audio/audiodev/opl.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <regstr.h>
#include <setupapi.h>
#include <ddk/ntddk.h>

namespace OPL {
namespace InpOut {

typedef void (__stdcall *Out32Proc)(short, short);
typedef short (__stdcall *Inp32Proc)(short);
typedef BOOL (__stdcall *IsInpOutDriverOpenProc)();
typedef BOOL (WINAPI *IsWow64ProcessProc)(HANDLE, PBOOL);

class OPL : public ::OPL::RealOPL {
public:
	OPL(Config::OplType type);
	~OPL();

	bool init();
	void reset();

	void write(int a, int v);
	byte read(int a);

	void writeReg(int r, int v);

private:
	void close();
	void delayReads(int reads);
	void delayTime(__int64 startTime, float delay);

	bool isWin64() const;

	Config::OplType _type;
	Out32Proc _outProc;
	Inp32Proc _inProc;
	IsInpOutDriverOpenProc _openProc;
	IsWow64ProcessProc _isWow64Proc;
	HINSTANCE _dll;
	__int64 _delayFreq;

	enum {
		// FIXME: This needs to be all configurable
		// Or detected. Or something.
		// 0x388 is common. Later revision CMI8738 cards use address space + 0x50
#if 1
		kFMPort = 0x388
#else
		kFMPort = 0xC050
#endif
	};
};

OPL::OPL(Config::OplType type) :
	_type(type),
	_outProc(0),
	_inProc(0),
	_openProc(0),
	_isWow64Proc(0),
	_dll(0),
	_delayFreq(0) {

	// Get the address of the IsWow64Process() function
	HMODULE kernel32Module = GetModuleHandleA("kernel32.dll");
	if (kernel32Module)
		_isWow64Proc = (IsWow64ProcessProc)GetProcAddress(kernel32Module, "IsWow64Process");
}

OPL::~OPL() {
	close();
}

bool OPL::init() {
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
		close();
		return false;
	}

	if (!_openProc()) {
		warning("Failed to initialize InpOut");
		close();
		return false;
	}

	// Calculate the CPU time for 1us
	LARGE_INTEGER tempFreq;
	QueryPerformanceFrequency(&tempFreq);
	_delayFreq = tempFreq.QuadPart / 1000000;

	// OPL2 detection
	writeReg(0x04, 0x60);
	writeReg(0x04, 0x80);
	byte status1 = read(kFMPort) & 0xE0;
	//debug("STATUS1: %02X", status1);

	writeReg(0x02, 0xFF);
	writeReg(0x04, 0x21);
	delayReads(80);
	byte status2 = read(kFMPort) & 0xE0;
	//debug("STATUS2: %02X", status2);

	writeReg(0x04, 0x60);
	writeReg(0x04, 0x80);

	if (status1 != 0x00 || status2 != 0xC0) {
		warning("Failed to detect OPL2");
		close();
		return false;
	}

	debug(1, "Detected OPL2!");

	if (_type != Config::kOpl2) {
		// OPL3 detection
		status1 = read(kFMPort) & 0x06;

		if (status1 != 0x00) {
			warning("Failed to detect OPL3");
			close();
			return false;
		}

		debug(1, "Detected OPL3!");
	}

	reset();

	return true;
}

void OPL::close() {
	_outProc = 0;
	_inProc = 0;
	_openProc = 0;

	if (_dll) {
		FreeLibrary(_dll);
		_dll = 0;
	}
}

void OPL::reset() {
	// OPL3 Enable
	if (_type == Config::kOpl2) {
		writeReg(0x105, 0x00);
	} else {
		writeReg(0x105, 0x01);
	}

	writeReg(0x001, 0x20); // Test Register
	writeReg(0x002, 0x20); // Timer 1
	writeReg(0x003, 0x00); // Timer 2
	writeReg(0x004, 0x00); // IRQ Mask Clear
	writeReg(0x104, 0x00); // 4-op mode disable
	writeReg(0x008, 0x00); // Keyboard split

	// Make sure all internal calculations finish sound generation
	for (int i = 0; i < 9; i++) {
		writeReg(0x0C0 | i, 0x00);
		writeReg(0x1C0 | i, 0x00);
	}

	for (int i = 0; i < 0x16; i++) {
		if ((i & 0x07) >= 0x06)
			continue;

		writeReg(0x040 | i, 0x3F);
		writeReg(0x140 | i, 0x3F);

		writeReg(0x080 | i, 0xFF);
		writeReg(0x180 | i, 0xFF);
		writeReg(0x060 | i, 0xFF);
		writeReg(0x160 | i, 0xFF);

		writeReg(0x020 | i, 0x00);
		writeReg(0x120 | i, 0x00);

		writeReg(0x0E0 | i, 0x00);
		writeReg(0x1E0 | i, 0x00);
	}

	writeReg(0x0BD, 0x00);

	for (int i = 0; i < 9; i++) {
		writeReg(0x0B0 | i, 0x00);
		writeReg(0x1B0 | i, 0x00);
		writeReg(0x0A0 | i, 0x00);
		writeReg(0x1A0 | i, 0x00);
	}

	for (int i = 0x40; i < 0xA0; i++) {
		if ((i & 0x07) >= 0x06 || (i & 0x1F) >= 0x18)
			continue;

		writeReg(0x000 | i, 0x00);
		writeReg(0x100 | i, 0x00);
	}

	for (int i = 0; i < 9; i++) {
		writeReg(0x0C0 | i, 0x30);
		writeReg(0x1C0 | i, 0x30);
	}
}

void OPL::write(int port, int val) {
	// Adjust port to be within our port
	port = kFMPort + (port & 3);

	// Write directly
	_outProc(port, val);
}

byte OPL::read(int port) {
	return _inProc(port);
}

void OPL::writeReg(int r, int v) {
	int port = kFMPort;
	if (r & 0x100)
		port += 2;

	LARGE_INTEGER startTime;
	QueryPerformanceCounter(&startTime);
	_outProc(port, r & 0xFF);
	delayTime(startTime.QuadPart, 0.00);

	QueryPerformanceCounter(&startTime);
	_outProc(port + 1, v);
	delayTime(startTime.QuadPart, 0.23);
}

void OPL::delayReads(int reads) {
	for (int i = 0; i < reads; i++)
		read(kFMPort);
}

void OPL::delayTime(__int64 startTime, float delay) {
	read(kFMPort);
	__int64 endTime = (__int64)(startTime + _delayFreq * delay);
	LARGE_INTEGER curTime;

	do {
		QueryPerformanceCounter(&curTime);
	} while (curTime.QuadPart < endTime);
}

bool OPL::isWin64() const {
#ifdef SCUMM_64BITS
	// If we're 64-bit, we have to be on Win64
	return true;
#else
	// We'll need to call the IsWow64Process() function
	// If not present, this is 32-bit Windows still
	if (!_isWow64Proc)
		return false;

	BOOL isWow64 = false;
	if (!_isWow64Proc(GetCurrentProcess(), &isWow64))
		return false;

	return isWow64;
#endif
}

OPL *create(Config::OplType type) {
	return new OPL(type);
}

} // End of namespace InpOut
} // End of namespace OPL

