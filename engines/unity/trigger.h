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

#ifndef UNITY_TRIGGER_H
#define UNITY_TRIGGER_H

#include "object.h"

namespace Unity {

enum {
	TRIGGERTYPE_NORMAL = 0x0,
	TRIGGERTYPE_TIMER = 0x1,
	TRIGGERTYPE_PROXIMITY = 0x2,
	TRIGGERTYPE_UNUSED = 0x3
};

struct Trigger {
	uint32 type;
	uint32 id;
	objectID target;
	bool enabled;
	byte unknown_a, unknown_b;
	uint32 timer_start;

	// timer
	uint32 target_time;

	// proximity
	uint16 dist;
	objectID from, to;
	bool reversed, instant;

	Trigger() : target_time(0) { }
	bool tick(UnityEngine *_vm);

protected:
	bool timerTick(UnityEngine *_vm);
	bool proximityTick(UnityEngine *_vm);
};

} // Unity

#endif
