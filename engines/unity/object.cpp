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

#include "object.h"
#include "conversation.h"
#include "unity.h"
#include "sprite_player.h"
#include "trigger.h"
#include "sound.h"
#include "graphics.h"

#include "common/stream.h"
#include "common/textconsole.h"

namespace Unity {

#define VERIFY_LENGTH(x) do { uint16 block_length = objstream->readUint16LE(); assert(block_length == x); } while (0)

objectID readObjectID(Common::SeekableReadStream *stream) {
	objectID r;
	stream->read(&r, 4);
	assert(r.unused == 0 || r.unused == 0xff);
	return r;
}

objectID readObjectIDBE(Common::SeekableReadStream *stream) {
	objectID r;
	stream->read(&r.unused, 1);
	stream->read(&r.world, 1);
	stream->read(&r.screen, 1);
	stream->read(&r.id, 1);
	assert(r.unused == 0 || r.unused == 0xff);
	return r;
}

Object::Object(UnityEngine *p) : _vm(p) {
	sprite = 0;
	flags = 0;
	timer = 0;
}

Object::~Object() {
	delete sprite;
}

// TODO: these are 'sanity checks' for verifying global state
bool g_debug_loading_object = false;
objectID g_debug_object_id;
extern unsigned int g_debug_conv_response, g_debug_conv_state;

Common::String Object::identify() {
	return Common::String::format("%s (%02x%02x%02x)", name.c_str(), id.world, id.screen, id.id);
}

void Object::loadObject(unsigned int for_world, unsigned int for_screen, unsigned int for_id) {
	Common::String filename = Common::String::format("o_%02x%02x%02x.bst", for_world, for_screen, for_id);
	Common::SeekableReadStream *objstream = _vm->data.openFile(filename);

	int type = readBlockHeader(objstream);
	if (type != BLOCK_OBJ_HEADER)
		error("bad block type %x encountered at start of object", type);
	VERIFY_LENGTH(0x128);

	id = readObjectID(objstream);
	assert(id.id == for_id);
	assert(id.screen == for_screen);
	assert(id.world == for_world);

	curr_screen = objstream->readByte();

	byte unknown3 = objstream->readByte(); // XXX: unused?
	debug(2, "unknown3:%d", unknown3);

	width = objstream->readSint16LE();
	height = objstream->readSint16LE();

	x = objstream->readSint16LE();
	y = objstream->readSint16LE();
	z = objstream->readSint16LE();
	universe_x = objstream->readSint16LE();
	universe_y = objstream->readSint16LE();
	universe_z = objstream->readSint16LE();

	// TODO: this doesn't work properly (see DrawOrderComparison)
	y_adjust = objstream->readSint16LE();

	anim_index = objstream->readUint16LE();

	sprite_id = objstream->readUint16LE();
	sprite = NULL;

	region_id = objstream->readUint32LE();

	flags = objstream->readByte();
	state = objstream->readByte();

	uint8 use_count = objstream->readByte();
	uint8 get_count = objstream->readByte();
	uint8 look_count = objstream->readByte();

	uint8 unknown11 = objstream->readByte(); // XXX
	assert(unknown11 == 0);

	objwalktype = objstream->readByte();
	assert(objwalktype <= OBJWALKTYPE_AS);

	uint8 description_count = objstream->readByte();

	char _name[20];
	objstream->read(_name, 20);
	name = _name;

	// pointers for DESCRIPTION, USE, LOOK, WALK and TIME,
	// which we probably don't care about?
	for (unsigned int i = 0; i < 5; i++) {
		uint16 unknowna = objstream->readUint16LE();
		byte unknownb = objstream->readByte();
		byte unknownc = objstream->readByte();
		if (i == 0) {
			// but we do want to snaffle this, lurking in the middle
			transition = readObjectID(objstream);
		}
		debug(2, "unknown(a:%d, b:%d, c:%d)", unknowna, unknownb, unknownc);
	}

	skills = objstream->readUint16LE();
	timer = objstream->readUint16LE();

	char _str[101];
	objstream->read(_str, 100);
	_str[100] = 0;
	setTalkString(_str);

	uint16 unknown19 = objstream->readUint16LE(); // XXX
	assert(unknown19 == 0x0);
	uint16 unknown20 = objstream->readUint16LE(); // XXX
	assert(unknown20 == 0x0);

	uint16 zero16 = objstream->readUint16LE();
	assert(zero16 == 0x0);

	uint16 unknown21 = objstream->readUint16LE(); // XXX: unused?
	debug(2, "unknown21: %d", unknown21);

	voice_id = objstream->readUint32LE();
	voice_group = objstream->readUint32LE();
	voice_subgroup = objstream->readUint16LE();

	cursor_id = objstream->readByte();
	cursor_flag = objstream->readByte();

	byte unknown26 = objstream->readByte(); // XXX
	debug(2, "unknown26: %d", unknown26);
	byte unknown27 = objstream->readByte(); // XXX
	debug(2, "unknown27: %d", unknown27);

	zero16 = objstream->readUint16LE();
	assert(zero16 == 0x0);

	for (unsigned int i = 0; i < 21; i++) {
		uint32 padding = objstream->readUint32LE();
		assert(padding == 0x0);
	}

	int blockType;
	while ((blockType = readBlockHeader(objstream)) != -1) {
		g_debug_object_id = id;
		g_debug_loading_object = true;
		readBlock(blockType, objstream);
		g_debug_loading_object = false;
	}

	assert(descriptions.size() == description_count);
	assert(use_entries.list.size() == use_count);
	assert(get_entries.list.size() == get_count);
	assert(look_entries.list.size() == look_count);
	assert(timer_entries.list.size() <= 1); // timers can only have one result

	delete objstream;
}

void Object::loadSprite() {
	if (sprite) return;

	if (sprite_id == 0xffff || sprite_id == 0xfffe) return;

	Common::String sprfilename = _vm->data.getSpriteFilename(sprite_id);
	sprite = new SpritePlayer(sprfilename.c_str(), this, _vm);
	sprite->startAnim(0); // XXX
}

int readBlockHeader(Common::SeekableReadStream *objstream) {
	byte type = objstream->readByte();
	if (objstream->eos()) return -1;

	byte header = objstream->readByte();
	assert(header == 0x11);

	return type;
}

void Object::readBlock(int type, Common::SeekableReadStream *objstream) {
	switch (type) {
		case BLOCK_DESCRIPTION:
			// technically these are always first, but we'll be forgiving
			readDescriptions(objstream);
			break;

		case BLOCK_USE_ENTRIES:
			use_entries.readEntryList(objstream);
			break;

		case BLOCK_GET_ENTRIES:
			get_entries.readEntryList(objstream);
			break;

		case BLOCK_LOOK_ENTRIES:
			look_entries.readEntryList(objstream);
			break;

		case BLOCK_TIMER_ENTRIES:
			timer_entries.readEntryList(objstream);
			break;

		default:
			// either an invalid block type, or it shouldn't be here
			error("bad block type %x encountered while parsing object", type);
	}
}

void EntryList::readEntryList(Common::SeekableReadStream *objstream) {
	byte num_entries = objstream->readByte();

	for (unsigned int i = 0; i < num_entries; i++) {
		int type = readBlockHeader(objstream);
		if (type != BLOCK_BEGIN_ENTRY)
			error("bad block type %x encountered while parsing entry", type);
		VERIFY_LENGTH(0xae);

		uint16 header = objstream->readUint16LE();
		assert(header == 9);

		// XXX: seeking over important data!
		// this seems to be offsets etc, unimportant?
		objstream->seek(0xac, SEEK_CUR);

		// XXX: horrible
		list.push_back(new Common::Array<Entry *>());

		while (true) {
			type = readBlockHeader(objstream);

			if (type == BLOCK_END_ENTRY)
				break;

			readEntry(type, objstream);
		}
	}
}

void Entry::readHeaderFrom(Common::SeekableReadStream *stream, byte header_type) {
	// in objects: this is the object id
	// in states: 'id' is the state id, the rest are zero?
	internal_obj.id = stream->readByte();
	internal_obj.screen = stream->readByte();
	internal_obj.world = stream->readByte();
	if (g_debug_loading_object) {
		assert(internal_obj.id == g_debug_object_id.id);
		assert(internal_obj.screen == g_debug_object_id.screen);
		assert(internal_obj.world == g_debug_object_id.world);
	} else {
		assert(internal_obj.id == g_debug_conv_state);
		assert(internal_obj.screen == 0);
		assert(internal_obj.world == 0);
	}

	// counters for offset within parent blocks (inner/outer)
	// TODO: add g_debug checks for these
	counter1 = stream->readByte();
	counter2 = stream->readByte();
	counter3 = stream->readByte();
	counter4 = stream->readByte();

	// misc header stuff
	byte h_type = stream->readByte();
	assert(h_type == header_type);

	stop_here = stream->readByte();
	assert(stop_here == 0 || stop_here == 1 || stop_here == 0xff);

	// the state/response of this conversation block?, or 0xffff in an object
	response_counter = stream->readUint16LE();
	state_counter = stream->readUint16LE();
	if (g_debug_loading_object) {
		assert(response_counter == 0xffff);
		assert(state_counter == 0xffff);
	} else {
		assert(response_counter == g_debug_conv_response);
		assert(state_counter == g_debug_conv_state);
	}
}

bool valid_world_id(uint16 world_id) {
	return world_id == 0x5f || world_id == 0x10 || world_id == 0 || (world_id >= 2 && world_id <= 7);
}

void ConditionBlock::readFrom(Common::SeekableReadStream *objstream) {
	readHeaderFrom(objstream, 0xff);

	// target
	target = readObjectID(objstream);
	assert(target.world == 0xff || valid_world_id(target.world));

	// WhoCan?
	WhoCan = readObjectID(objstream);
	assert(WhoCan.world == 0xff || valid_world_id(WhoCan.world));

	for (unsigned int i = 0; i < 4; i++) {
		condition[i] = readObjectID(objstream);
		assert(condition[i].world == 0xff || valid_world_id(condition[i].world));

		check_x[i] = objstream->readUint16LE();
		check_y[i] = objstream->readUint16LE();
		check_unknown[i] = objstream->readUint16LE(); // unused check_z?
		check_univ_x[i] = objstream->readUint16LE();
		check_univ_y[i] = objstream->readUint16LE();
		check_univ_z[i] = objstream->readUint16LE();
		check_screen[i] = objstream->readByte();
		check_status[i] = objstream->readByte();
		check_state[i] = objstream->readByte();

		switch (i) {
			case 0:
			case 1:
				assert(check_univ_x[i] == 0xffff);
				assert(check_univ_y[i] == 0xffff);
				assert(check_univ_z[i] == 0xffff);
				break;
			case 2:
			case 3:
				assert(check_x[i] == 0xffff);
				assert(check_y[i] == 0xffff);
				assert(check_unknown[i] == 0xffff);
				assert(check_univ_x[i] == 0xffff);
				assert(check_univ_y[i] == 0xffff);
				assert(check_univ_z[i] == 0xffff);
				break;
		}
	}

	uint16 unknown1 = objstream->readUint16LE();
	assert(unknown1 == 0xffff);
	uint16 unknown2 = objstream->readUint16LE();
	assert(unknown2 == 0xffff);
	uint16 unknown3 = objstream->readUint16LE();
	assert(unknown3 == 0xffff);

	how_close_x = objstream->readUint16LE();
	how_close_y = objstream->readUint16LE();

	uint16 unknown4 = objstream->readUint16LE();
	assert(unknown4 == 0xffff);

	how_close_dist = objstream->readUint16LE();

	// TODO: zero skill check becomes -1 at load
	skill_check = objstream->readUint16LE();

	counter_value = objstream->readUint16LE();
	counter_when = objstream->readByte();

	for (unsigned int i = 0; i < 25; i++) {
		uint32 zero = objstream->readUint32LE();
		assert(zero == 0);
	}
}

void AlterBlock::readFrom(Common::SeekableReadStream *objstream) {
	readHeaderFrom(objstream, 0x43);

	target = readObjectID(objstream);

	alter_flags = objstream->readByte();
	alter_reset = objstream->readByte();

	alter_timer = objstream->readUint16LE(); // set timer..

	uint16 unknown16 = objstream->readUint16LE();
	assert(unknown16 == 0xffff);

	alter_anim = objstream->readUint16LE();
	alter_state = objstream->readByte();

	play_description = objstream->readByte();

	x_pos = objstream->readUint16LE();
	y_pos = objstream->readUint16LE();
	// z_pos?
	unknown8 = objstream->readUint16LE();

	// these are always 0xffff inside objects..
	universe_x = objstream->readUint16LE();
	universe_y = objstream->readUint16LE();
	universe_z = objstream->readUint16LE();

	char text[101];
	objstream->read(text, 20);
	text[20] = 0;
	alter_name = text;
	objstream->read(text, 100);
	text[100] = 0;
	alter_hail = text;

	uint32 unknown32 = objstream->readUint32LE();
	assert(unknown32 == 0xffffffff);

	voice_id = objstream->readUint32LE();
	voice_group = objstream->readUint32LE();
	voice_subgroup = objstream->readUint16LE();
	// the voice_subgroup is not always changed..
	assert(voice_group != 0xffffffff || voice_subgroup == 0xffff);
	// .. neither is the voice_id
	assert(voice_group != 0xffffffff || voice_id == 0xffffffff);

	// talk begin/end stuff??
	unknown11 = objstream->readByte();
	unknown12 = objstream->readByte();
	// random checks
	assert(unknown11 == 0xff || unknown11 == 0 || unknown11 == 1 || unknown11 == 2 || unknown11 == 3);
	assert(unknown12 == 0xff || unknown12 == 0 || unknown12 == 1);

	// zeros
	for (unsigned int i = 0; i < 21; i++) {
		unknown32 = objstream->readUint32LE();
		assert(unknown32 == 0);
	}
}

void ReactionBlock::readFrom(Common::SeekableReadStream *objstream) {
	readHeaderFrom(objstream, 0x44);

	target = readObjectID(objstream);
	dest_world = objstream->readUint16LE();
	dest_screen = objstream->readUint16LE();
	dest_entrance = objstream->readUint16LE();
	target_type = objstream->readByte();
	action_type = objstream->readByte();
	damage_amount = objstream->readByte();
	beam_type = objstream->readByte();
	dest_x = objstream->readUint16LE();
	dest_y = objstream->readUint16LE();
	dest_unknown = objstream->readUint16LE();

	assert(target_type >= 1 && target_type <= 7);
	if (target_type != 6) {
		assert(target.id == 0xff);
	} else {
		assert(target.id != 0xff);
	}
	if (damage_amount == 0) {
		assert(action_type <= 3);
		// otherwise it can be 0, 1 or ff, but afaik just junk
	}

	for (unsigned int i = 0; i < 100; i++) {
		byte unknown = objstream->readByte();
		assert(unknown == 0);
	}
}

void CommandBlock::readFrom(Common::SeekableReadStream *objstream) {
	readHeaderFrom(objstream, 0x45);

	target[0] = readObjectID(objstream);
	target[1] = readObjectID(objstream);
	target[2] = readObjectID(objstream);

	// both usually 0xffff
	target_x = objstream->readUint16LE();
	target_y = objstream->readUint16LE();

	command_id = objstream->readUint32LE();

	assert(command_id >= 2 && command_id <= 6);

	for (unsigned int i = 0; i < 97; i++) {
		byte unknown = objstream->readByte();
		assert(unknown == 0);
	}
}

void ScreenBlock::readFrom(Common::SeekableReadStream *objstream) {
	readHeaderFrom(objstream, 0x46);

	new_screen = objstream->readByte();
	new_entrance = objstream->readByte();
	advice_screen = objstream->readByte();
	new_advice_id = objstream->readUint16LE();
	new_world = objstream->readUint16LE();

	unknown6 = objstream->readUint16LE();
	unknown7 = objstream->readUint32LE();
	unknown8 = objstream->readByte();
	unknown9 = objstream->readUint32LE();
	unknown10 = objstream->readUint32LE();
	unknown11 = objstream->readUint16LE();
	assert(unknown11 == 0xffff);
	unknown12 = objstream->readUint16LE();
	assert(unknown12 == 0xffff);

	unknown13 = objstream->readUint16LE();
	unknown14 = objstream->readByte();
	unknown15 = objstream->readByte();
	unknown16 = objstream->readByte();

	new_advice_timer = objstream->readUint16LE();

	for (unsigned int i = 0; i < 24; i++) {
		uint32 zero = objstream->readUint32LE();
		assert(zero == 0);
	}
}

void PathBlock::readFrom(Common::SeekableReadStream *objstream) {
	readHeaderFrom(objstream, 0x47);

	objstream->seek(0x16c, SEEK_CUR); // XXX
}

void GeneralBlock::readFrom(Common::SeekableReadStream *objstream) {
	readHeaderFrom(objstream, 0x48);

	movie_id = objstream->readUint16LE();

	unknown1 = objstream->readUint16LE(); // XXX
	unknown2 = objstream->readUint16LE(); // XXX
	unknown3 = objstream->readUint16LE(); // XXX

	for (unsigned int i = 0; i < 0x64; i++) {
		byte zero = objstream->readByte();
		assert(zero == 0);
	}
}

void ConversationBlock::readFrom(Common::SeekableReadStream *objstream) {
	readHeaderFrom(objstream, 0x49);

	world_id = objstream->readUint16LE();
	assert(world_id == 0xffff || valid_world_id(world_id));
	conversation_id = objstream->readUint16LE();
	response_id = objstream->readUint16LE();
	state_id = objstream->readUint16LE();

	action_id = objstream->readUint16LE();
	assert(action_id == RESPONSE_ENABLED || action_id == RESPONSE_DISABLED ||
		action_id == RESPONSE_UNKNOWN1);

	for (unsigned int i = 0; i < 0x63; i++) {
		byte zero = objstream->readByte();
		assert(zero == 0);
	}
}

void BeamBlock::readFrom(Common::SeekableReadStream *objstream) {
	readHeaderFrom(objstream, 0x4a);

	// world id, or 00 to kill?
	world_id = objstream->readUint16LE();

	// unused?
	unknown1 = objstream->readUint16LE();
	assert(unknown1 == 0xffff || unknown1 == 1 || unknown1 == 4);

	uint16 unknown2 = objstream->readUint16LE();
	assert(unknown2 == 0xffff);
	unknown2 = objstream->readUint16LE();
	assert(unknown2 == 0xffff);
	unknown2 = objstream->readUint16LE();
	assert(unknown2 == 0xffff);

	// unused?
	unknown3 = objstream->readUint16LE();
	assert(unknown3 == 0 || unknown3 == 0xffff);

	unknown2 = objstream->readUint16LE();
	assert(unknown2 == 0xffff);
	unknown2 = objstream->readUint16LE();
	assert(unknown2 == 0xffff);
	unknown2 = objstream->readUint16LE();
	assert(unknown2 == 0xffff);

	screen_id = objstream->readUint16LE();
	assert(screen_id <= 4 || screen_id == 0xffff);

	// from here this is all unused junk, i think
	uint16 unknown5 = objstream->readUint16LE();
	uint16 unknown6 = objstream->readUint16LE();
	assert(unknown5 == 0xffff || unknown5 == 0 || unknown5 == 0x60);
	assert(unknown6 == 0xffff || unknown6 == 0x20 || unknown6 == 0x1b);
	for (unsigned int i = 0; i < 3; i++) {
		unknown2 = objstream->readUint16LE();
		assert(unknown2 == 0x0);
		uint32 unknown32 = objstream->readUint32LE();
		assert(unknown32 == 0xffffffff);
	}
	for (unsigned int i = 0; i < 51; i++) {
		unknown2 = objstream->readUint16LE();
		assert(unknown2 == 0x0);
	}
}

void TriggerBlock::readFrom(Common::SeekableReadStream *objstream) {
	readHeaderFrom(objstream, 0x4b);

	trigger_id = objstream->readUint32LE();

	byte flag = objstream->readByte();
	enable_trigger = (flag == 1);

	for (unsigned int i = 0; i < 25; i++) {
		uint32 unknown32 = objstream->readUint32LE();
		assert(unknown32 == 0);
	}
}

void CommunicateBlock::readFrom(Common::SeekableReadStream *objstream) {
	readHeaderFrom(objstream, 0x4c);

	target = readObjectID(objstream);

	if (target.id == 0xff) {
		assert(target.screen == 0xff);
		assert(target.world == 0xff);
		assert(target.unused == 0xff);
	}

	conversation_id = objstream->readUint16LE();
	situation_id = objstream->readUint16LE();
	hail_type = objstream->readByte();
	assert(hail_type <= 0x10);

	for (unsigned int i = 0; i < 25; i++) {
		uint32 unknown32 = objstream->readUint32LE();
		assert(unknown32 == 0);
	}
}

void ChoiceBlock::readFrom(Common::SeekableReadStream *objstream) {
	readHeaderFrom(objstream, 0x4d);

	// TODO: x/y?
	_unknown1 = objstream->readUint16LE();
	_unknown2 = objstream->readUint16LE();

	char buffer[101];
	buffer[100] = 0;
	objstream->read(buffer, 100);
	_questionstring = buffer;

	buffer[16] = 0;
	objstream->read(buffer, 16);
	_choicestring[0] = buffer;
	objstream->read(buffer, 16);
	_choicestring[1] = buffer;

	_object = readObjectID(objstream);

	// offsets and counts for the choices, it seems; we don't need this
	uint32 offset1 = objstream->readUint32LE();
	uint32 offset2 = objstream->readUint32LE();
	uint16 count1 = objstream->readUint16LE();
	uint16 count2 = objstream->readUint16LE();
	(void)offset1; (void)offset2; (void)count1; (void)count2;

	for (unsigned int i = 0; i < 25; i++) {
		uint32 unknown32 = objstream->readUint32LE();
		assert(unknown32 == 0);
	}
}

void EntryList::readEntry(int type, Common::SeekableReadStream *objstream) {
	uint16 header;

	// XXX: horrible
	Common::Array<Entry *> &entries = *list[list.size() -1 ];

	switch (type) {
		case BLOCK_CONDITION:
			VERIFY_LENGTH(0xda);
			header = objstream->readUint16LE();
			assert(header == 9);

			{
			ConditionBlock *block = new ConditionBlock();
			block->readFrom(objstream);
			entries.push_back(block);
			}
			break;

		case BLOCK_ALTER:
			while (true) {
				VERIFY_LENGTH(0x105);
				header = objstream->readUint16LE();
				assert(header == 9);

				AlterBlock *block = new AlterBlock();
				block->readFrom(objstream);
				entries.push_back(block);

				type = readBlockHeader(objstream);
				if (type == BLOCK_END_BLOCK)
					break;
				if (type != BLOCK_ALTER)
					error("bad block type %x encountered while parsing alters", type);
			}
			break;

		case BLOCK_REACTION:
			while (true) {
				VERIFY_LENGTH(0x87);
				header = objstream->readUint16LE();
				assert(header == 9);

				ReactionBlock *block = new ReactionBlock();
				block->readFrom(objstream);
				entries.push_back(block);

				type = readBlockHeader(objstream);
				if (type == BLOCK_END_BLOCK)
					break;
				if (type != BLOCK_REACTION)
					error("bad block type %x encountered while parsing reactions", type);
			}
			break;

		case BLOCK_COMMAND:
			while (true) {
				VERIFY_LENGTH(0x84);
				header = objstream->readUint16LE();
				assert(header == 9);

				CommandBlock *block = new CommandBlock();
				block->readFrom(objstream);
				entries.push_back(block);

				type = readBlockHeader(objstream);
				if (type == BLOCK_END_BLOCK)
					break;
				if (type != BLOCK_COMMAND)
					error("bad block type %x encountered while parsing commands", type);
			}
			break;

		case BLOCK_SCREEN:
			VERIFY_LENGTH(0x90);
			header = objstream->readUint16LE();
			assert(header == 9);
			{
			ScreenBlock *block = new ScreenBlock();
			block->readFrom(objstream);
			entries.push_back(block);
			}
			break;

		case BLOCK_PATH:
			VERIFY_LENGTH(0x17b);
			header = objstream->readUint16LE();
			assert(header == 9);
			{
			PathBlock *block = new PathBlock();
			block->readFrom(objstream);
			entries.push_back(block);
			}
			break;

		case BLOCK_GENERAL:
			VERIFY_LENGTH(0x7b);
			header = objstream->readUint16LE();
			assert(header == 9);
			{
			GeneralBlock *block = new GeneralBlock();
			block->readFrom(objstream);
			entries.push_back(block);
			}
			break;

		case BLOCK_CONVERSATION:
			while (true) {
				VERIFY_LENGTH(0x7c);
				header = objstream->readUint16LE();
				assert(header == 9);

				ConversationBlock *block = new ConversationBlock();
				block->readFrom(objstream);
				entries.push_back(block);

				type = readBlockHeader(objstream);
				if (type == BLOCK_END_BLOCK)
					break;
				if (type != BLOCK_CONVERSATION)
					error("bad block type %x encountered while parsing conversation", type);
			}
			break;

		case BLOCK_BEAMDOWN:
			VERIFY_LENGTH(0x9f);
			header = objstream->readUint16LE();
			assert(header == 9);
			{
			BeamBlock *block = new BeamBlock();
			block->readFrom(objstream);
			entries.push_back(block);
			}
			break;

		case BLOCK_TRIGGER:
			while (true) {
				VERIFY_LENGTH(0x78);
				header = objstream->readUint16LE();
				assert(header == 9);

				TriggerBlock *block = new TriggerBlock();
				block->readFrom(objstream);
				entries.push_back(block);

				type = readBlockHeader(objstream);
				if (type == BLOCK_END_BLOCK)
					break;
				if (type != BLOCK_TRIGGER)
					error("bad block type %x encountered while parsing triggers", type);
			}
			break;

		case BLOCK_COMMUNICATE:
			VERIFY_LENGTH(0x7c);
			header = objstream->readUint16LE();
			assert(header == 9);
			{
			CommunicateBlock *block = new CommunicateBlock();
			block->readFrom(objstream);
			entries.push_back(block);
			}
			break;

		case BLOCK_CHOICE:
			VERIFY_LENGTH(0x10b);
			header = objstream->readUint16LE();
			assert(header == 9);
			{
			ChoiceBlock *block = new ChoiceBlock();
			block->readFrom(objstream);
			entries.push_back(block);

			while (true) {
				type = readBlockHeader(objstream);
				if (type == BLOCK_END_BLOCK)
					break;

				if (type == BLOCK_CHOICE1) {
					type = readBlockHeader(objstream);
					// XXX: horrible
					block->_choice[0].list.push_back(new Common::Array<Entry *>());
					block->_choice[0].readEntry(type, objstream);
				} else if (type == BLOCK_CHOICE2) {
					type = readBlockHeader(objstream);
					// XXX: horrible
					block->_choice[1].list.push_back(new Common::Array<Entry *>());
					block->_choice[1].readEntry(type, objstream);
				} else
					error("bad block type %x encountered while parsing choices", type);
			}
			}
			break;

		default:
			error("bad block type %x encountered while parsing entries", type);
	}
}

void Object::readDescriptions(Common::SeekableReadStream *objstream) {
	readDescriptionBlock(objstream);

	while (true) {
		int type = readBlockHeader(objstream);
		switch (type) {
			case BLOCK_DESCRIPTION:
				readDescriptionBlock(objstream);
				break;

			case BLOCK_END_BLOCK:
				return;

			default:
				error("bad block type %x encountered while parsing object", type);
		}
	}
}

void Object::readDescriptionBlock(Common::SeekableReadStream *objstream) {
	VERIFY_LENGTH(0xa5);

	byte entry_for = objstream->readByte();

	for (unsigned int i = 0; i < 7; i++) {
		uint16 unknowna = objstream->readUint16LE();
		assert(unknowna == 0xffff);
	}

	char text[150];
	objstream->read(text, 149);
	text[149] = 0;

	// XXX: is this just corrupt entries and there should be a null byte here?
	byte unknown2 = objstream->readByte();
	debug(2, "unknown2: %d", unknown2);

	int type = readBlockHeader(objstream);
	if (type != BLOCK_SPEECH_INFO)
		error("bad block type %x encountered while parsing description", type);
	VERIFY_LENGTH(0x0c);

	Description desc;
	desc.text = text;
	desc.entry_id = entry_for;

	desc.voice_id = objstream->readUint32LE();
	desc.voice_group = objstream->readUint32LE();
	desc.voice_subgroup = objstream->readUint32LE();

	descriptions.push_back(desc);
}

void Object::changeTalkString(const Common::String &str) {
	bool immediate = (str.size() && str[0] == '1');
	if (immediate) {
		// run the conversation immediately, don't change anything
		// TODO: this should be *queued* to be run once we're done (or be wiped by fail)
		// TODO: voice stuff
		runHail(str);
	} else {
		debug(1, "talk string of %s changing from '%s' to '%s'", identify().c_str(),
			talk_string.c_str(), str.c_str());
		setTalkString(str);
	}
}

void Object::setTalkString(const Common::String &str) {
	if (!str.size()) {
		talk_string.clear();
		return;
	}

	if (str[0] == '1') error("unexpected immediate talk string '%s'", str.c_str());

	// TODO: 'abc     ---hello---' should be stripped to 'abchello'
	// (skip two sets of dashes, then spaces)

	talk_string = str;
}

void Object::runHail(const Common::String &hail) {
	debug(1, "%s running hail '%s'", identify().c_str(), hail.c_str());

	// TODO: check OBJFLAG_ACTIVE?
	// TODO: use source of change if an away team member

	if (hail.size() < 2) {
		error("failed to parse hail '%s'", hail.c_str());
	}

	bool immediate = (hail[0] == '1');
	if ((immediate && hail[1] != '@') || (!immediate && hail[0] != '@')) {
		_vm->_dialog_text = hail.c_str() + (immediate ? 1 : 0);

		// TODO: this is VERY not good
		_vm->setSpeaker(id);
		debug(1, "%s says '%s'", identify().c_str(), hail.c_str());

		_vm->runDialog();

		// text, not a proper hail
		return;
	}

	int world, conversation, situation;
	if (sscanf(hail.begin() + (immediate ? 2 : 1), "%d,%d,%d",
		&world, &conversation, &situation) != 3) {
		// try two-parameter form (with default world)
		world = _vm->data._currentScreen.world;
		if (sscanf(hail.begin() + (immediate ? 2 : 1), "%d,%d",
			&conversation, &situation) != 2) {
			error("failed to parse hail '%s'", hail.c_str());
		}
	}

	// TODO: not always Picard! :(
	objectID speaker = objectID(0, 0, 0);
	if (_vm->_current_away_team_member) speaker = _vm->_current_away_team_member->id;

	Conversation *conv = _vm->data.getConversation(world, conversation);
	conv->execute(_vm, _vm->data.getObject(speaker), situation);
}

EntryList::~EntryList() {
	for (unsigned int i = 0; i < list.size(); i++) {
		for (unsigned int j = 0; j < list[i]->size(); j++) {
			delete (*list[i])[j];
		}
		delete list[i];
	}
}

ResultType EntryList::execute(UnityEngine *_vm, Action *context) {
	debugN(1, "\n");
	ResultType r = 0;
	for (unsigned int i = 0; i < list.size(); i++) {
		debug(1, "EntryList::execute: block %d of %d (size %d)", i + 1, list.size(), list[i]->size());
		bool run = true;
		r &= (RESULT_WALKING | RESULT_MATCHOTHER | RESULT_DIDSOMETHING);
		for (unsigned int j = 0; j < list[i]->size(); j++) {
			r |= (*list[i])[j]->check(_vm, context);
			if (r & ~(RESULT_MATCHOTHER | RESULT_DIDSOMETHING)) {
				run = false;
				break;
			}
		}
		if (!run) continue;
		for (unsigned int j = 0; j < list[i]->size(); j++) {
			r |= (*list[i])[j]->execute(_vm, context);
			r |= RESULT_DIDSOMETHING;
			if ((*list[i])[j]->stop_here) {
				r |= RESULT_STOPPED;
				debug(1, "EntryList::execute: stopped at entry %d of %d", j + 1, list[i]->size());
				break;
			}
		}
		if (r & RESULT_STOPPED) break;
	}
	debug(1, "EntryList::execute: done with %d blocks", list.size());
	debugN(1, "\n");
	return r;
}

ResultType ConditionBlock::check(UnityEngine *_vm, Action *context) {
	debug(1, "ConditionBlock::check: %02x%02x%02x, %02x%02x%02x",
		target.world, target.screen, target.id,
		WhoCan.world, WhoCan.screen, WhoCan.id);

	ResultType r = 0;

	if (target.world == 0xff && target.screen == 0xff && target.id == 0xfe) {
		// fail if other was passed in
		debugN("(checking if other was set) ");
		if (context->other.id != 0xff) {
			debugN("-- it was!\n");
			return RESULT_FAILOTHER;
		}
	} else if (target.world != 0xff && target.screen != 0xff && target.id != 0xff) {
		// fail if source != other
		debugN("(checking if target %02x%02x%02x matches other %02x%02x%02x) ",
			target.world, target.screen, target.id,
			context->other.world, context->other.screen, context->other.id);
		if (target != context->other) {
			debugN("-- it doesn't!\n");
			return RESULT_FAILOTHER;
		}
		r |= RESULT_MATCHOTHER;
	}

	if (how_close_dist != 0xffff) {
		// TODO: fail if `who' object doesn't exist?
		uint16 x = how_close_x;
		uint16 y = how_close_y;
		if (x == 0xffff) {
			// TODO: if source has OBJFLAG_INVENTORY, take `who' position
			// otherwise, take source position
		}
		// TODO: if squared distance between `who' and x/y > how_close_dist*how_close_dist {
		//	run walk action with `who', return RESULT_WALKING|RESULT_DIDSOMETHING
		// }
		// TODO: (wth? checking `who' vs `who'?)
		warning("unimplemented: ConditionCheck: ignoring HowClose(x: %d y: %d)", x, y);
	}

	if ((WhoCan.id != 0xff) &&
		!((WhoCan.world == 0) && (WhoCan.screen) == 0 && (WhoCan.id == 0x10))) {
		debugN("(checking if WhoCan %02x%02x%02x matches who %02x%02x%02x) ",
			WhoCan.world, WhoCan.screen, WhoCan.id,
			context->who.world, context->who.screen, context->who.id);
		if (WhoCan != context->who) {
			debugN(" -- nope\n");
			return r | RESULT_AWAYTEAM;
		}
	}

	// TODO: skill level of who or RESULT_FAILSKILL

	if (counter_when != 0xff) {
		// counter_flag = 1;

		if (counter_when == 1) {
			// do when
			if (counter_value) {
				counter_value--;
				debugN("counter: not yet\n");
				return r | RESULT_COUNTER_DOWHEN;
			}
		} else if (counter_when == 0) {
			// do until
			if (counter_value == 0xffff) {
				debugN("counter: not any more\n");
				return r | RESULT_COUNTER_DOUNTIL;
			}
		}
		counter_value--;
	}

	bool did_something = false;
	for (unsigned int i = 0; i < 4; i++) {
		// TODO: some special handling of check_x/check_y for invalid obj?
		if (condition[i].world == 0xff) continue;

		Object *obj;
		if (condition[i].world == 0 && condition[i].screen == 0 && condition[i].id == 0x10) {
			debugN("(got away team member) ");
			// TODO: what to do if not on away team?
			obj = _vm->_current_away_team_member;
		} else {
			obj = _vm->data.getObject(condition[i]);
		}
		debugN("checking state of %s", obj->identify().c_str());

		if (check_state[i] != 0xff) {
			debugN(" (is state %x?)", check_state[i]);
			if (obj->state != check_state[i]) {
				debugN(" -- nope!\n");
				return r | RESULT_FAILSTATE;
			}
		}

		if (!(obj->flags & OBJFLAG_ACTIVE)) {
			if (check_screen[i] == 0xfe) {
				// check for inactive
				return r;
			}

			if (check_state[i] == 0xff) {
				debugN(" (object inactive!)\n");
				return r | RESULT_INACTIVE;
			}
		}

		// TODO: check whether stunned or not?

		if (check_status[i] != 0xff) {
			did_something = true;

			if (check_status[i]) {
				if (obj->id.world == 0 && obj->id.screen == 0 && obj->id.id < 0x10) {
					debugN(" (in away team?)");
					if (Common::find(_vm->_away_team_members.begin(),
						_vm->_away_team_members.end(),
						obj) == _vm->_away_team_members.end()) {
						debugN(" -- nope!\n");
						return r | RESULT_AWAYTEAM;
					}
				} else {
					debugN(" (in inventory?)");
					if (!(obj->flags & OBJFLAG_INVENTORY)) {
						debugN(" -- nope!\n");
						return r | RESULT_INVENTORY;
					}
				}
			} else {
				// note that there is no inverse away team check
				debugN(" (not in inventory?)");
				if (obj->flags & OBJFLAG_INVENTORY) {
					debugN(" -- it is!\n");
					return r | RESULT_INVENTORY;
				}
			}
		}

		if (check_x[i] != 0xffff) {
			did_something = true;
			debugN(" (is x/y %x/%x?)", check_x[i], check_y[i]);
			if (obj->x != check_x[i] || obj->y != check_y[i]) {
				debugN(" -- nope!\n");
				return r | RESULT_FAILPOS;
			}
		}

		// (check_unknown unused?)

		if (check_univ_x[i] != 0xffff) {
			did_something = true;
			debugN(" (is universe x/y/z %x/%x/%x?)", check_univ_x[i], check_univ_y[i], check_univ_z[i]);
			if (obj->universe_x != check_univ_x[i] || obj->universe_y != check_univ_y[i] ||
				obj->universe_z != check_univ_z[i]) {
				debugN(" -- nope!\n");
				return r | RESULT_FAILPOS;
			}
		}

		if (check_screen[i] != 0xff) {
			did_something = true;
			debugN(" (is screen %x?)", check_screen[i]);

			if (obj->curr_screen != check_screen[i]) {
				debugN(" -- nope!\n");
				return r | RESULT_FAILSCREEN;
			}
		}

		debugN("\n");
	}

	// TODO: stupid hardcoded tricorder sound

	return r;
}

ResultType ConditionBlock::execute(UnityEngine *_vm, Action *context) {
	return 0; // nothing to do
}

ResultType AlterBlock::execute(UnityEngine *_vm, Action *context) {
	debug(1, "Alter: on %02x%02x%02x", target.world, target.screen, target.id);

	Object *obj;
	if (target.world == 0 && target.screen == 0 && target.id == 0x10) {
		// TODO: use 'who' from context
		obj = _vm->data.getObject(objectID(0, 0, 0));
	} else {
		obj = _vm->data.getObject(target);
	}

	bool did_something = false; // just for debugging

	if (alter_flags || alter_reset) {
		did_something = true;

		// magical flags!

		if (alter_reset & 0x01) {
			// activate
			debug(1, "AlterBlock::execute (%s): activating", obj->identify().c_str());
			obj->flags |= OBJFLAG_ACTIVE;
		}
		if (alter_reset & ~0x01) {
			warning("unimplemented: AlterBlock::execute (%s): alter_reset %x", obj->identify().c_str(), alter_reset);
		}

		if (alter_flags & 0x1) {
			// TODO: talk? - run immediately (action 5), who is worldobj+16(??)
			warning("unimplemented: AlterBlock::execute (%s): alter_flag 1", obj->identify().c_str());
		}
		if (alter_flags & 0x02) {
			error("invalid AlterBlock flag 0x02");
		}
		if (alter_flags & 0x4) {
			// drop
			debug(1, "AlterBlock::execute (%s): dropping", obj->identify().c_str());
			obj->flags &= ~OBJFLAG_INVENTORY;

			if (Common::find(_vm->_inventory_items.begin(),
				_vm->_inventory_items.end(), obj) != _vm->_inventory_items.end()) {
				_vm->removeFromInventory(obj);
			}
		}
		if (alter_flags & 0x8) {
			// get
			debug(1, "AlterBlock::execute (%s): getting", obj->identify().c_str());
			if (!(obj->flags & OBJFLAG_INVENTORY)) {
				obj->flags |= (OBJFLAG_ACTIVE | OBJFLAG_INVENTORY);
			}

			if (Common::find(_vm->_inventory_items.begin(),
				_vm->_inventory_items.end(), obj) == _vm->_inventory_items.end()) {
				_vm->addToInventory(obj);
			}
		}
		if (alter_flags & 0x10) {
			// unstun
			debug(1, "AlterBlock::execute (%s): unstunning", obj->identify().c_str());
			// TODO: special behaviour on away team?
			obj->flags &= ~OBJFLAG_STUNNED;
		}
		if (alter_flags & 0x20) {
			// stun
			debug(1, "AlterBlock::execute (%s): stunning", obj->identify().c_str());
			// TODO: special behaviour on away team?
			obj->flags |= OBJFLAG_STUNNED;
		}
		if (alter_flags & 0x40) {
			// TODO: ?? (anim change?)
			warning("unimplemented: AlterBlock::execute (%s): alter_flag 0x40", obj->identify().c_str());
		}
		if (alter_flags & 0x80) {
			// deactivate
			debug(1, "AlterBlock::execute (%s): deactivating", obj->identify().c_str());
			obj->flags &= ~(OBJFLAG_ACTIVE | OBJFLAG_INVENTORY);
		}
	}

	if (x_pos != 0xffff && y_pos != 0xffff) {
		did_something = true;
		debug(1, "AlterBlock::execute (%s): position (%x, %x)", obj->identify().c_str(), x_pos, y_pos);

		obj->x = x_pos;
		obj->y = y_pos;
		// TODO: z_pos too?
	}

	if (alter_name.size()) {
		did_something = true;
		debug(1, "altering name of %s to %s", obj->identify().c_str(), alter_name.c_str());

		obj->name = alter_name;
	}

	if (alter_hail.size()) {
		did_something = true;

		obj->changeTalkString(alter_hail);
	}

	if (voice_group != 0xffffffff || voice_subgroup != 0xffff || voice_id != 0xffffffff) {
		did_something = true;
		warning("unimplemented: AlterBlock::execute (%s): voice %x/%x/%x", obj->identify().c_str(), voice_group, voice_subgroup, voice_id);

		// TODO: alter voice for the description? set all, if voice_id is not 0xffffffff
	}

	if (alter_state != 0xff) {
		did_something = true;
		debug(1, "AlterBlock::execute (%s): state %x", obj->identify().c_str(), alter_state);

		obj->state = alter_state;
	}

	if (alter_timer != 0xffff) {
		did_something = true;
		debug(1, "AlterBlock::execute (%s): timer %x", obj->identify().c_str(), alter_timer);

		obj->timer = alter_timer;
	}

	if (alter_anim != 0xffff) {
		did_something = true;
		debug(1, "AlterBlock::execute (%s): anim %04x", obj->identify().c_str(), alter_anim);

		uint16 anim_id = alter_anim;

		if (anim_id >= 31000) {
			// TODO (some magic with (alter_anim - 31000))
			// (this sets a different animation type - index?)
			// this doesn't run ANY of the code below
			anim_id -= 31000;
			error("very weird animation id %04x (%d)", alter_anim, anim_id);
		} else if (anim_id >= 29000) {
			if (alter_anim < 30000) {
				anim_id = 30000 - alter_anim;
				// TODO: some other magic?
				// (try going into the third area of Allanor: 020412 tries this on the drone)
				// TODO: this probably just force-stops the animation
				warning("weird animation id %04x (%d)", alter_anim, anim_id);
			} else {
				// TODO: ?!?
				anim_id -= 30000;
			}
		}

		if (obj->sprite) {
			if (obj->sprite->getNumAnims() <= anim_id) {
				// use the Chodak transporter bar gauge, for example
				warning("animation %d exceeds %d?!", anim_id, obj->sprite->getNumAnims());
			} else {
				obj->sprite->startAnim(anim_id);
			}
		} else if (obj->id == objectID(0, 1, 0x5f)) {
			// sprite change on the Enterprise
			_vm->_viewscreen_sprite_id = alter_anim;
		} else {
			warning("no sprite?!");
		}
	}

	if (play_description != 0) {
		did_something = true;
		// TODO: is the value of play_description meaningful?
		_vm->playDescriptionFor(obj);
	}

	if (unknown8 != 0xffff) {
		did_something = true;
		// TODO: set y_adjust
		warning("unimplemented: AlterBlock::execute (%s): unknown8 %x", obj->identify().c_str(), unknown8);
	}

	if (universe_x != 0xffff || universe_y != 0xffff || universe_z != 0xffff) {
		did_something = true;
		debug(1, "AlterBlock::execute (%s): warp to universe loc %x, %x, %x", obj->identify().c_str(), universe_x, universe_y, universe_z);

		if (obj->id.world == 0x5f && obj->id.screen == 1 && obj->id.id == 0) {
			// TODO: go into astrogation and start warp to this universe location
			warning("unimplemented: Enterprise warp");
		}
		obj->universe_x = universe_x;
		obj->universe_y = universe_y;
		obj->universe_z = universe_z;
	}

	if (unknown11 != 0xff) {
		// TODO: cursor id
		did_something = true;
		warning("unimplemented: AlterBlock::execute (%s): unknown11 %x", obj->identify().c_str(), unknown11);
	}

	if (unknown12 != 0xff) {
		// TODO: cursor flag
		did_something = true;
		warning("unimplemented: AlterBlock::execute (%s): unknown12 %x", obj->identify().c_str(), unknown12);
	}

	if (!did_something) {
		warning("empty AlterBlock::execute (%s)?", obj->identify().c_str());
	}

	return 0;
}

ResultType ReactionBlock::execute(UnityEngine *_vm, Action *context) {
	warning("unimplemented: ReactionBlock::execute");

	Common::Array<Object *> objects;

	switch (target_type - 1) {
	case 0: // who from context
		// TODO
		break;

	case 2: // all away team members (plus stunned members on current screen?)
		// TODO
		break;

	case 5: // provided object
		// TODO: force onto current screen?
		objects.push_back(_vm->data.getObject(target));
		break;

	case 6: // random away team member
		// TODO
		break;

	default: error("bad reaction target type %d", target_type);
	}

	if (damage_amount == 0) {
		switch (action_type) {
		case 0: // stun all
			// TODO: set health to 1 below minimum
			break;

		case 1: // kill all
			// TODO: set health to 0
			break;

			// TODO: for below, beam_type is 0 for normal (teffect/beamout/beamin),
			// 1 for shodak (chodbeam/acid/acid)

		case 2: // beam in all to dest_x/dest_y
			// TODO:
			break;

		case 3: if (dest_world == 0) {
				// TODO: beam out to ship
			} else {
				// TODO: ensure that dest_world is 0xffff or current world, since
				// you can't beam between worlds

				if (dest_entrance == 0xffff) error("no destination entrance in reaction");

				// TODO: if dest_screen is current screen, beam in to dest_entrance
				// TODO: else, beam out to dest_entrance
			}
			break;
		default: error("bad reaction action type %d", action_type);
		}
	} else if (damage_amount == 0x7f) {
		// heal all objects to 1 above minimum health
		// TODO
	} else {
		// do damage_amount of damage to all objects
		// TODO
		// TODO: if beam_type is set, do phaser effects too
	}

	return 0;
}

ResultType CommandBlock::execute(UnityEngine *_vm, Action *context) {
	debug(1, "CommandBlock: %02x%02x%02x/%02x%02x%02x/%02x%02x%02x, (%d, %d), command %d",
		target[0].world, target[0].screen, target[0].id,
		target[1].world, target[1].screen, target[1].id,
		target[2].world, target[2].screen, target[2].id,
		target_x, target_y, command_id);

	ActionType action_id;
	switch (command_id) {
		case 2: action_id = ACTION_LOOK; break;
		case 3: action_id = ACTION_GET; break;
		case 4: action_id = ACTION_WALK; break;
		case 5: action_id = ACTION_TALK; break;
		case 6: action_id = ACTION_USE; break;
		default: error("unknown command type %x", command_id);
	}

	Object *targ = NULL;
	if (target[0].world != 0xff)
		targ = _vm->data.getObject(target[0]);

	_vm->performAction(action_id, targ, target[1], target[2], target_x, target_y);

	return 0;
}

ResultType ScreenBlock::execute(UnityEngine *_vm, Action *context) {
	if (new_screen != 0xff) {
		debug(1, "ScreenBlock::execute: new screen: %02x, entrance %02x", new_screen, new_entrance);

		byte entrance = new_entrance;
		// TODO: if new_entrance not -1 and matches 0x40, weird stuff with away team?
		// (forcing into source region, etc)
		if (entrance != 0xff) entrance &= ~0x40;
		// TODO: 0x80 can be set too.. see exit from first Unity Device screen
		if (entrance != 0xff) entrance &= ~0x80;

		_vm->startAwayTeam(_vm->data._currentScreen.world, new_screen, entrance);
	} else
		warning("unimplemented: ScreenBlock::execute");

	if (advice_screen == 0xff) {
		if (new_advice_id != 0xffff) {
			// TODO: live change of new_advice_id and new_advice_timer for current screen right now
		}
	} else {
		// TODO: set new_advice_id for target screen
		if (new_advice_timer != 0xffff) {
			// TODO: set new_advice_timer for target screen
		}
	}

	if (new_world != 0xffff) {
		// TODO: this is just a guess
		debug(1, "ScreenBlock::execute: new_world (back to bridge?): %04x", new_world);
		_vm->startBridge();
	}

	// TODO: screen to bump?
	if (unknown6 != 0xffff) debugN ("6: %04x ", unknown6);

	// unused??
	if (unknown7 != 0xffffffff) debugN ("7: %08x ", unknown7);
	if (unknown8 != 0xff) debugN ("8: %02x ", unknown8);
	if (unknown9 != 0xffffffff) debugN ("9: %08x ", unknown9);
	if (unknown10 != 0xffffffff) debugN ("10: %08x ", unknown10);
	if (unknown11 != 0xffff) debugN ("11: %04x ", unknown11);
	if (unknown12 != 0xffff) debugN ("12: %04x ", unknown12);

	// TODO: screen id + changes for screen?
	if (unknown13 != 0xffff) debugN ("13: %04x ", unknown13);
	if (unknown14 != 0xff) debugN ("14: %02x ", unknown14);
	if (unknown15 != 0xff) debugN ("15: %02x ", unknown15);
	if (unknown16 != 0xff) debugN ("16: %02x ", unknown16);

	debugN("\n");

	return 0;
}

ResultType PathBlock::execute(UnityEngine *_vm, Action *context) {
	warning("unimplemented: PathBlock::execute");

	return 0;
}

ResultType GeneralBlock::execute(UnityEngine *_vm, Action *context) {
	if (unknown1 != 0xffff) {
		warning("unimplemented: GeneralBlock::execute: unknown1 %04x", unknown1);
	}
	if (unknown2 != 0) {
		warning("unimplemented: GeneralBlock::execute: unknown2 %04x", unknown2);
	}
	if (unknown3 != 0xffff) {
		warning("unimplemented: GeneralBlock::execute: unknown3 %04x", unknown3);
	}
	if (movie_id != 0xffff) {
		assert(_vm->data._movieFilenames.contains(movie_id));

		debug(1, "GeneralBlock: play movie %d (%s: '%s')", movie_id,
			_vm->data._movieFilenames[movie_id].c_str(),
			_vm->data._movieDescriptions[movie_id].c_str());

		_vm->_gfx->playMovie(_vm->data._movieFilenames[movie_id]);
	}

	return 0;
}

ResultType ConversationBlock::execute(UnityEngine *_vm, Action *context) {
	debug(1, "ConversationBlock::execute: @0x%02x,%d,%d,%d: action %d",
		world_id, conversation_id, response_id, state_id, action_id);

	uint16 world = world_id;
	if (world == 0xffff) world = _vm->data._currentScreen.world;

	Conversation *conv = _vm->data.getConversation(world, conversation_id);
	Response *resp = conv->getResponse(response_id, state_id);
	resp->response_state = action_id;

	return 0;
}

ResultType BeamBlock::execute(UnityEngine *_vm, Action *context) {
	debug(1, "BeamBlock::execute with %04x, %04x", world_id, screen_id);

	_vm->_beam_world = world_id;
	_vm->_beam_screen = screen_id;

	return 0;
}

ResultType TriggerBlock::execute(UnityEngine *_vm, Action *context) {
	Trigger *trigger = _vm->data.getTrigger(trigger_id);

	debug(1, "triggerBlock: trying to set trigger %x to %d", trigger_id, enable_trigger);
	trigger->enabled = enable_trigger;

	return 0;
}

ResultType CommunicateBlock::execute(UnityEngine *_vm, Action *context) {
	debug(1, "CommunicateBlock::execute: at %02x%02x%02x, %04x, %04x, %02x", target.world, target.screen, target.id, conversation_id, situation_id, hail_type);

	Object *targ;
	// TODO: not Picard!! what are we meant to do here?
	if (target.id == 0xff) {
		targ = _vm->data.getObject(objectID(0, 0, 0));
	} else {
		targ = _vm->data.getObject(target);
	}

	if (hail_type != 0xff && hail_type != 0x7 && hail_type != 0x8) {
		if (target.id != 0xff) {
			// must be a non-immediate hail
			if (!targ->talk_string.size() || targ->talk_string[0] != '@')
				error("'%s' is not a valid hail string (during Communicate)",
					targ->talk_string.c_str());

			// TODO: use hail_type!!
			targ->runHail(targ->talk_string);
			return 0;
		}
	}

	switch (hail_type) {
	case 0:
		// 0: we are being hailed (on screen)
	case 1:
		// 1: subspace frequency (open channels)
	case 2:
		// 2: hailed by planet (on screen)
	case 3:
		// 3: beacon (open channels)
		warning("unhandled optional hail (%02x)", hail_type);
		break;
	case 7:
		// 7: delayed conversation, oddness with 0xFE and 0x00/0x5F checks
		//    (elsewhere: special case conversation 99?!?)
		// on-viewscreen?
		_vm->changeToScreen(ViewscreenScreenType);
		// fallthrough..
	case 4:
		// 4: delayed conversation? (forced 0x5f)
	case 5:
		// 5: delayed conversation? (forced 0x5f)
	case 6:
		// 6: immediate conversation
	case 8:
		// 8: delayed conversation? (forced 0x5f)
		_vm->_next_conversation = _vm->data.getConversation(_vm->data._currentScreen.world,
			conversation_id);

		// original engine simply ignores this when there are no enabled situations, it seems
		// (TODO: check what happens when there is no such situation at all)
		// TODO: which speaker?? not Picard :(
		if (_vm->_next_conversation->getEnabledResponse(_vm, situation_id, objectID(0, 0, 0))) {
			// this overrides any existing conversations.. possibly that is a good thing
			_vm->_next_situation = situation_id;
		} else _vm->_next_situation = 0xffffffff;
		break;

	case 9:
		// 9: reset conversation to none
	case 0xa:
		// a: reset conversation to none
		warning("unhandled special hail (%02x)", hail_type);
		break;
	case 0xb:
		// b: magicB + some storing of obj screen/id + wiping of that obj + reset conversation?
		// (back to bridge?)
		_vm->changeToScreen(BridgeScreenType);
		break;
	case 0xc:
		// c: magicB + reset conversation?
		// (some bridge change?)
	case 0xd:
		// d: reset conversation (but not obj) + magic?
		// (some bridge change? more complex than the others)
	case 0xe:
		// e: do nothing?
	case 0xf:
		// f: magicF + reset conversation?
		// (some *different* bridge change?)
	case 0x10:
		// 0x10: magicB + reset conversation + magic?
		warning("unhandled special hail (%02x)", hail_type);
	}

	return 0;
}

ResultType ChoiceBlock::execute(UnityEngine *_vm, Action *context) {
	warning("unimplemented: ChoiceBlock::execute");

	return 0;
}

} // Unity

