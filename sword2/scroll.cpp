/* Copyright (C) 1994-2004 Revolution Software Ltd
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
#include "sword2/interpreter.h"
#include "sword2/logic.h"

namespace Sword2 {

// Max no of pixel allowed to scroll per cycle
#define MAX_SCROLL_DISTANCE 8

/**
 * If the room is larger than the physical screen, this function is called
 * every game cycle to update the scroll offsets.
 */

void Sword2Engine::setScrolling(void) {
	// Normally we aim to get George's feet at (320,250) from top left
	// of screen window
	// feet_x = 128 + 320
	// feet_y = 128 + 250

	// Set scroll offsets according to the player's coords

	// If the scroll offsets are being forced in script, ensure that they
	// are neither too far to the right nor too far down.

	if (Logic::_scriptVars[SCROLL_X] || Logic::_scriptVars[SCROLL_Y]) {
		_thisScreen.scroll_offset_x = MIN((uint16) Logic::_scriptVars[SCROLL_X], _thisScreen.max_scroll_offset_x);
		_thisScreen.scroll_offset_y = MIN((uint16) Logic::_scriptVars[SCROLL_Y], _thisScreen.max_scroll_offset_y);
		return;
	}

	// George's offset from the centre - the desired position for him

	int16 offset_x = _thisScreen.player_feet_x - _thisScreen.feet_x;
	int16 offset_y = _thisScreen.player_feet_y - _thisScreen.feet_y;

	// Prevent scrolling too far left/right/up/down

	if (offset_x < 0)
		offset_x = 0;
	else if (offset_x > _thisScreen.max_scroll_offset_x)
		offset_x = _thisScreen.max_scroll_offset_x;

	if (offset_y < 0)
		offset_y = 0;
	else if (offset_y > _thisScreen.max_scroll_offset_y)
		offset_y = _thisScreen.max_scroll_offset_y;

	// First time on this screen - need absolute scroll immediately!

	if (_thisScreen.scroll_flag == 2) {
		debug(5, "init scroll");
		_thisScreen.scroll_offset_x = offset_x;
		_thisScreen.scroll_offset_y = offset_y;
		_thisScreen.scroll_flag = 1;
		return;
	}

	// Catch up with required scroll offsets - speed depending on distance
	// to catch up (dx and dy) and _scrollFraction used, but limit to
	// certain number of pixels per cycle (MAX_SCROLL_DISTANCE)

	int16 dx = _thisScreen.scroll_offset_x - offset_x;
	int16 dy = _thisScreen.scroll_offset_y - offset_y;

	uint16 scroll_distance_x;	// how much we want to scroll
	uint16 scroll_distance_y;

	if (dx < 0) {
		// Current scroll_offset_x is less than the required value

		// NB. I'm adding 1 to the result of dx / SCROLL_FRACTION,
		// because it would otherwise not scroll at all when
		// dx < SCROLL_FRACTION

		// => inc by (fraction of the differnce) NB. dx is -ve, so we
		// subtract dx / SCROLL_FRACTION

		scroll_distance_x = 1 - dx / _scrollFraction;

		if (scroll_distance_x > MAX_SCROLL_DISTANCE)
			scroll_distance_x = MAX_SCROLL_DISTANCE;

		_thisScreen.scroll_offset_x += scroll_distance_x;
	} else if (dx > 0) {
		// Current scroll_offset_x is greater than
		// the required value

		// => dec by (fraction of the differnce)

		scroll_distance_x = 1 + dx / _scrollFraction;

		if (scroll_distance_x > MAX_SCROLL_DISTANCE)
			scroll_distance_x = MAX_SCROLL_DISTANCE;

		_thisScreen.scroll_offset_x -= scroll_distance_x;
	}

	if (dy < 0) {
		scroll_distance_y = 1 - dy / _scrollFraction;

		if (scroll_distance_y > MAX_SCROLL_DISTANCE)
			scroll_distance_y = MAX_SCROLL_DISTANCE;

		_thisScreen.scroll_offset_y += scroll_distance_y;
	} else if (dy > 0) {
		scroll_distance_y = 1 + dy / _scrollFraction;

		if (scroll_distance_y > MAX_SCROLL_DISTANCE)
			scroll_distance_y = MAX_SCROLL_DISTANCE;

		_thisScreen.scroll_offset_y -= scroll_distance_y;
	}
}

/**
 * Set the special scroll offset variables
 *
 * Call when starting screens and to change the camera within screens
 *
 * call AFTER fnInitBackground() to override the defaults
 */

int32 Logic::fnSetScrollCoordinate(int32 *params) {
	// params:	0 feet_x value
	// 		1 feet_y value

  	// Called feet_x and feet_y to retain intellectual compatibility with
	// Sword1!
	//
	// feet_x & feet_y refer to the physical screen coords where the
	// system will try to maintain George's feet

	_vm->_thisScreen.feet_x = params[0];
	_vm->_thisScreen.feet_y = params[1];
	return IR_CONT;
}

int32 Logic::fnSetScrollSpeedNormal(int32 *params) {
	// params:	none

	_vm->_scrollFraction = 16;
	return IR_CONT;
}

int32 Logic::fnSetScrollSpeedSlow(int32 *params) {
	// params:	none

	_vm->_scrollFraction = 32;
	return IR_CONT;
}

} // End of namespace Sword2
