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

#ifndef UNITY_UNITY_H
#define UNITY_UNITY_H

#include "engines/engine.h"
#include "common/random.h"

#include "unity/console.h"

#include "data.h"

namespace Unity {

class Graphics;
class Sound;
class SpritePlayer;
class Object;
class Trigger;
class UIScreen;

enum AwayTeamMode {
	mode_Look,
	mode_Use,
	mode_Walk,
	mode_Talk
};

// Engine Debug Flags
enum {
	kDebugResource  = (1 << 0),
	kDebugSaveLoad  = (1 << 1),
	kDebugScript    = (1 << 2),
	kDebugGraphics  = (1 << 3),
	kDebugSound     = (1 << 4)
};

enum ScreenType {
	NoScreenType,
	AstrogationScreenType,
	BridgeScreenType,
	ComputerScreenType,
	DeathScreenType,
	EngineeringScreenType,
	HolodeckScreenType,
	TacticalScreenType,
	TransporterScreenType,
	ViewscreenScreenType
};

class UnityEngine : public Engine {
public:
	UnityEngine(class OSystem *syst);
	~UnityEngine();

	Common::Error init();
	Common::Error run();
        GUI::Debugger *getDebugger() { return _console; }

	Object *objectAt(unsigned int x, unsigned int y);
	void clearObjects();
	void removeObject(Object *obj);

	Common::RandomSource *_rnd;

	UnityData data;

	Sound *_snd;
	Graphics *_gfx;

	bool _on_away_team;
	AwayTeamMode _mode;

	Object *_current_away_team_member;
	SpritePlayer *_current_away_team_icon;
	Common::Array<Object *> _away_team_members;
	Common::Array<Object *> _inventory_items;
	Common::Array<SpritePlayer *> _inventory_icons;
	unsigned int _inventory_index;
	void addToInventory(Object *obj);
	void removeFromInventory(Object *obj);

	Common::String _status_text;

	unsigned int _dialog_x, _dialog_y;
	bool _in_dialog;
	bool _dialog_choosing;
	Common::String _dialog_text;
	Common::Array<Common::String> _choice_list;
	void setSpeaker(objectID s);

	// TODO: horrible hack
	unsigned int _dialog_choice_situation;
	Common::Array<unsigned int> _dialog_choice_states;

	Conversation *_next_conversation;
	unsigned int _next_situation;

	uint16 _beam_world, _beam_screen;

	unsigned int _viewscreen_sprite_id;

	unsigned int runDialogChoice(Conversation *conversation);
	void runDialog();

	void startBridge();
	void endAwayTeam();
	void startAwayTeam(unsigned int world, unsigned int screen, byte entrance = 0);

	ResultType performAction(ActionType action_type, Object *target, objectID who = objectID(), objectID other = objectID(), unsigned int target_x = 0xffff, unsigned int target_y = 0xffff);

	void playDescriptionFor(Object *obj);
	Common::String voiceFileFor(byte voice_group, byte voice_subgroup, objectID speaker, byte voice_id, char type = 0);

	void changeToScreen(ScreenType screenType);

protected:
	UnityConsole *_console;

	objectID _speaker;
	SpritePlayer *_icon;

	UIScreen *_currScreen;
	ScreenType _currScreenType;
	class BridgeScreen *_bridgeScreen;
	class ComputerScreen *_computerScreen;
	class ViewscreenScreen *_viewscreenScreen;

	void openLocation(unsigned int world, unsigned int screen);

	void checkEvents();
	void handleAwayTeamMouseMove(const Common::Point &pos);
	void handleAwayTeamMouseClick(const Common::Point &pos);

	void drawObjects();
	void processTriggers();
	void processTimers();

	void startupScreen();

	uint _dialogSelected;
	uint _dialogStartLine;
	Common::Array<Common::Array<Common::String> > _dialogLines;
	Common::Array<Common::Rect> _dialogRects;
	uint _dialogWidth;
	void drawDialogFrameAround(unsigned int x, unsigned int y, unsigned int width,
		unsigned int height, bool use_thick_frame, bool with_icon, bool with_buttons);
	void initDialog();
	void dialogMouseMove(const Common::Point &pos);
	void dialogMouseClick(const Common::Point &pos);
	void drawDialogWindow();

	void drawAwayTeamUI();

	void handleLook(Object *obj);
	void handleUse(Object *obj);
	void handleWalk(Object *obj);
	void handleTalk(Object *obj);

	void DebugNextScreen();
};

} // Unity

#endif

