/* Copyright (C) 1994-1998 Revolution Software Ltd.
 * Copyright (C) 2003-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "common/stdafx.h"
#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/console.h"
#include "sword2/interpreter.h"
#include "sword2/logic.h"
#include "sword2/memory.h"
#include "sword2/resman.h"

#define Debug_Printf _vm->_debugger->DebugPrintf

namespace Sword2 {

void Logic::sendEvent(uint32 id, uint32 interact_id) {
	for (int i = 0; i < MAX_events; i++) {
		if (_eventList[i].id == id || !_eventList[i].id) {
			_eventList[i].id = id;
			_eventList[i].interact_id = interact_id;
			return;
		}
	}

	error("sendEvent() ran out of event slots");
}

void Logic::setPlayerActionEvent(uint32 id, uint32 interact_id) {
	// Full script id of action script number 2
	sendEvent(id, (interact_id << 16) | 2);
}

int Logic::checkEventWaiting(void) {
	for (int i = 0; i < MAX_events; i++) {
		if (_eventList[i].id == _scriptVars[ID])
			return 1;
	}

	return 0;
}

void Logic::startEvent(void) {
	// call this from stuff like fnWalk
	// you must follow with a return IR_TERMINATE

	for (int i = 0; i < MAX_events; i++) {
		if (_eventList[i].id == _scriptVars[ID]) {
			logicOne(_eventList[i].interact_id);
			_eventList[i].id = 0;
			return;
		}
	}

	error("startEvent() can't find event for id %d", _scriptVars[ID]);
}

void Logic::clearEvent(uint32 id) {
	for (int i = 0; i < MAX_events; i++) {
		if (_eventList[i].id == id) {
			_eventList[i].id = 0;
			return;
		}
	}
}

void Logic::killAllIdsEvents(uint32 id) {
	for (int i = 0; i < MAX_events; i++) {
		if (_eventList[i].id == id)
			_eventList[i].id = 0;
	}
}

// For the debugger

uint32 Logic::countEvents(void) {
	uint32 count = 0;

	for (int i = 0; i < MAX_events; i++) {
		if (_eventList[i].id)
			count++;
	}

	return count;
}

void Logic::printEventList(void) {
	Debug_Printf("EVENT LIST:\n");

	for (uint32 i = 0; i < MAX_events; i++) {
		if (_eventList[i].id) {
			byte buf[NAME_LEN];
			uint32 target = _eventList[i].id;
			uint32 script = _eventList[i].interact_id;

			Debug_Printf("slot %2d: id = %s (%d)\n", i, _vm->fetchObjectName(target, buf), target);
			Debug_Printf("         script = %s (%d) pos %d\n", _vm->fetchObjectName(script / 65536, buf), script / 65536, script % 65536);
		}
	}
}

} // End of namespace Sword2
