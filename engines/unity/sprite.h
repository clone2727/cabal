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

#ifndef UNITY_SPRITE_H
#define UNITY_SPRITE_H

#include "common/stream.h"
#include "common/array.h"

// XXX: is this always true?
#define COLOUR_BLANK 13

namespace Unity {

enum SpriteEntryType {
	se_None,
	se_SpeechSprite,
	se_Sprite,
	se_Palette,
	se_RandomWait,
	se_Wait,
	se_Pause,
	se_Jump,
	se_Position,
	se_RelPos,
	se_MouthPos,
	se_Mark,
	se_Mask,
	se_Static,
	se_Audio,
	se_WaitForSound,
	se_Silent,
	se_StateSet,
	se_SetFlag,
	se_Exit
};

struct SpriteEntry {
	SpriteEntryType type;
	SpriteEntry(SpriteEntryType _t) : type(_t) { }
};

struct SpriteEntrySprite : public SpriteEntry {
	unsigned int width;
	unsigned int height;
	byte *data;
	SpriteEntrySprite() : SpriteEntry(se_Sprite), data(0) { }
};

struct SpriteEntryPalette : public SpriteEntry {
	byte palette[256*3];
	SpriteEntryPalette() : SpriteEntry(se_Palette) { }
};

struct SpriteEntryWait : public SpriteEntry {
	unsigned int wait;
	SpriteEntryWait(unsigned int _w) : SpriteEntry(se_Wait), wait(_w) { }
};

struct SpriteEntryRandomWait : public SpriteEntry {
	unsigned int rand_amt, base;
	SpriteEntryRandomWait(unsigned int _r, unsigned int _b) : SpriteEntry(se_RandomWait), rand_amt(_r), base(_b) { }
};

struct SpriteEntryJump : public SpriteEntry {
	unsigned int target;
	SpriteEntryJump(unsigned int _t) : SpriteEntry(se_Jump), target(_t) { }
};

struct SpriteEntryPosition : public SpriteEntry {
	int newx, newy;
	SpriteEntryPosition(int _ax, int _ay) : SpriteEntry(se_Position), newx(_ax), newy(_ay) { }
};

struct SpriteEntryRelPos : public SpriteEntry {
	int adjustx, adjusty;
	SpriteEntryRelPos(int _ax, int _ay) : SpriteEntry(se_RelPos), adjustx(_ax), adjusty(_ay) { }
};

struct SpriteEntryMouthPos : public SpriteEntry {
	int adjustx, adjusty;
	SpriteEntryMouthPos(int _ax, int _ay) : SpriteEntry(se_MouthPos), adjustx(_ax), adjusty(_ay) { }
};

struct SpriteEntryAudio : public SpriteEntry {
	unsigned int length;
	byte *data;
	SpriteEntryAudio() : SpriteEntry(se_Audio), data(0) { }
};

struct SpriteEntryStateSet : public SpriteEntry {
	uint32 state;
	SpriteEntryStateSet(uint32 _s) : SpriteEntry(se_StateSet), state(_s) { }
};

struct SpriteEntrySetFlag : public SpriteEntry {
	uint32 flagId;
	SpriteEntrySetFlag(uint32 _f) : SpriteEntry(se_SetFlag), flagId(_f) { }
};


class Sprite {
public:
	Sprite(Common::SeekableReadStream *_str);
	~Sprite();

	SpriteEntry *getEntry(unsigned int entry) { return _entries[entry]; }
	unsigned int getNumEntries() { return _entries.size(); }
	unsigned int getIndexFor(unsigned int anim) { return _indexes[anim]; }
	unsigned int getNumAnims() { return _indexes.size(); }

protected:
	Common::SeekableReadStream *_stream;

	Common::Array<unsigned int> _indexes;
	Common::Array<SpriteEntry *> _entries;

	SpriteEntry *readBlock();
	SpriteEntry *parseBlock(char blockType[4], uint32 size);

	void readCompressedImage(uint32 size, SpriteEntrySprite *dest);
	void decodeSpriteTypeOne(byte *buf, unsigned int size, byte *data, unsigned int width, unsigned int height);
	void decodeSpriteTypeTwo(byte *buf, unsigned int size, byte *data, unsigned int targetsize);

	bool _isSprite;
};

} // Unity

#endif

