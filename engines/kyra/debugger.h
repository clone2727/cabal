/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef KYRA_DEBUGGER_H
#define KYRA_DEBUGGER_H

#include "gui/debugger.h"

namespace Kyra {

class KyraEngine;
class KyraEngine_v1;
class KyraEngine_HoF;
class KyraEngine_MR;

class Debugger : public ::GUI::Debugger {
public:
	Debugger(KyraEngine *vm);
	virtual ~Debugger() {}  // we need this for __SYMBIAN32__ archaic gcc/UIQ

protected:
	KyraEngine *_vm;

	bool cmd_setScreenDebug(int argc, const char **argv);
	bool cmd_loadPalette(int argc, const char **argv);
	bool cmd_showFacings(int argc, const char **argv);
	bool cmd_gameSpeed(int argc, const char **argv);
	bool cmd_listFlags(int argc, const char **argv);
	bool cmd_toggleFlag(int argc, const char **argv);
	bool cmd_queryFlag(int argc, const char **argv);
	bool cmd_listTimers(int argc, const char **argv);
	bool cmd_setTimerCountdown(int argc, const char **argv);
};

class Debugger_v1 : public Debugger {
public:
	Debugger_v1(KyraEngine_v1 *vm);
	virtual ~Debugger_v1() {}  // we need this for __SYMBIAN32__ archaic gcc/UIQ

protected:
	KyraEngine_v1 *_vm;

	virtual void preEnter();
	virtual void postEnter();

	bool cmd_enterRoom(int argc, const char **argv);
	bool cmd_listRooms(int argc, const char **argv);
	bool cmd_giveItem(int argc, const char **argv);
	bool cmd_listBirthstones(int argc, const char **argv);
};

class Debugger_v2 : public Debugger {
public:
	Debugger_v2(KyraEngine_HoF *vm);
	virtual ~Debugger_v2() {}

protected:
	KyraEngine_HoF *_vm;

	bool cmd_enterScene(int argc, const char **argv);
	bool cmd_listScenes(int argc, const char **argv);
	bool cmd_sceneInfo(int argc, const char **argv);
	bool cmd_characterInfo(int argc, const char **argv);
	bool cmd_sceneToFacing(int argc, const char **argv);
	bool cmd_giveItem(int argc, const char **argv);
	bool cmd_passcodes(int argc, const char **argv);
};

class Debugger_v3 : public Debugger {
public:
	Debugger_v3(KyraEngine_MR *vm);
	virtual ~Debugger_v3() {}

protected:
	KyraEngine_MR *_vm;
	
	bool cmd_giveItem(int argc, const char **argv);	
	bool cmd_enterScene(int argc, const char **argv);
	bool cmd_listScenes(int argc, const char **argv);
	bool cmd_sceneInfo(int argc, const char **argv);
	bool cmd_sceneToFacing(int argc, const char **argv);
};


} // End of namespace Kyra

#endif

