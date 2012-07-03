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
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/dcgf.h"
#include "engines/wintermute/Base/BSprite.h"
#include "engines/wintermute/utils/StringUtil.h"
#include "engines/wintermute/utils/PathUtil.h"
#include "engines/wintermute/Base/BParser.h"
#include "engines/wintermute/Base/BDynBuffer.h"
#include "engines/wintermute/Base/BSurface.h"
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/BFrame.h"
#include "engines/wintermute/Base/BSound.h"
#include "engines/wintermute/Base/BSubFrame.h"
#include "engines/wintermute/Base/BFileManager.h"
#include "engines/wintermute/PlatformSDL.h"
#include "engines/wintermute/Base/scriptables/ScValue.h"
#include "engines/wintermute/Base/scriptables/ScScript.h"
#include "engines/wintermute/Base/scriptables/ScStack.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CBSprite, false)

//////////////////////////////////////////////////////////////////////
CBSprite::CBSprite(CBGame *inGame, CBObject *Owner): CBScriptHolder(inGame) {
	_editorAllFrames = false;
	_owner = Owner;
	SetDefaults();
}


//////////////////////////////////////////////////////////////////////
CBSprite::~CBSprite() {
	cleanup();
}


//////////////////////////////////////////////////////////////////////////
void CBSprite::SetDefaults() {
	_currentFrame = -1;
	_looping = false;
	_lastFrameTime = 0;
	_filename = NULL;
	_finished = false;
	_changed = false;
	_paused = false;
	_continuous = false;
	_moveX = _moveY = 0;

	_editorMuted = false;
	_editorBgFile = NULL;
	_editorBgOffsetX = _editorBgOffsetY = 0;
	_editorBgAlpha = 0xFF;
	_streamed = false;
	_streamedKeepLoaded = false;

	setName("");

	_precise = true;
}


//////////////////////////////////////////////////////////////////////////
void CBSprite::cleanup() {
	CBScriptHolder::cleanup();

	for (int i = 0; i < _frames.GetSize(); i++)
		delete _frames[i];
	_frames.RemoveAll();

	delete[] _editorBgFile;
	_editorBgFile = NULL;

	SetDefaults();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSprite::Draw(int X, int Y, CBObject *Register, float ZoomX, float ZoomY, uint32 Alpha) {
	GetCurrentFrame(ZoomX, ZoomY);
	if (_currentFrame < 0 || _currentFrame >= _frames.GetSize()) return S_OK;

	// move owner if allowed to
	if (_changed && _owner && _owner->_movable) {
		_owner->_posX += _moveX;
		_owner->_posY += _moveY;
		_owner->afterMove();

		X = _owner->_posX;
		Y = _owner->_posY;
	}

	// draw frame
	return display(X, Y, Register, ZoomX, ZoomY, Alpha);
}


//////////////////////////////////////////////////////////////////////
HRESULT CBSprite::loadFile(const char *Filename, int LifeTime, TSpriteCacheType CacheType) {
	Common::SeekableReadStream *File = Game->_fileManager->openFile(Filename);
	if (!File) {
		Game->LOG(0, "CBSprite::LoadFile failed for file '%s'", Filename);
		if (Game->_dEBUG_DebugMode) return loadFile("invalid_debug.bmp", LifeTime, CacheType);
		else return loadFile("invalid.bmp", LifeTime, CacheType);
	} else {
		Game->_fileManager->closeFile(File);
		File = NULL;
	}

	HRESULT ret;

	AnsiString ext = PathUtil::GetExtension(Filename);
	if (StringUtil::StartsWith(Filename, "savegame:", true) || StringUtil::CompareNoCase(ext, "bmp") || StringUtil::CompareNoCase(ext, "tga") || StringUtil::CompareNoCase(ext, "png") || StringUtil::CompareNoCase(ext, "jpg")) {
		CBFrame *frame = new CBFrame(Game);
		CBSubFrame *subframe = new CBSubFrame(Game);
		subframe->setSurface(Filename, true, 0, 0, 0, LifeTime, true);
		if (subframe->_surface == NULL) {
			Game->LOG(0, "Error loading simple sprite '%s'", Filename);
			ret = E_FAIL;
			delete frame;
			delete subframe;
		} else {
			CBPlatform::SetRect(&subframe->_rect, 0, 0, subframe->_surface->getWidth(), subframe->_surface->getHeight());
			frame->_subframes.Add(subframe);
			_frames.Add(frame);
			_currentFrame = 0;
			ret = S_OK;
		}
	} else {
		byte *Buffer = Game->_fileManager->readWholeFile(Filename);
		if (Buffer) {
			if (FAILED(ret = loadBuffer(Buffer, true, LifeTime, CacheType))) Game->LOG(0, "Error parsing SPRITE file '%s'", Filename);
			delete [] Buffer;
		}
	}

	_filename = new char [strlen(Filename) + 1];
	strcpy(_filename, Filename);


	return ret;
}



TOKEN_DEF_START
TOKEN_DEF(CONTINUOUS)
TOKEN_DEF(SPRITE)
TOKEN_DEF(LOOPING)
TOKEN_DEF(FRAME)
TOKEN_DEF(NAME)
TOKEN_DEF(PRECISE)
TOKEN_DEF(EDITOR_MUTED)
TOKEN_DEF(STREAMED_KEEP_LOADED)
TOKEN_DEF(STREAMED)
TOKEN_DEF(SCRIPT)
TOKEN_DEF(EDITOR_BG_FILE)
TOKEN_DEF(EDITOR_BG_OFFSET_X)
TOKEN_DEF(EDITOR_BG_OFFSET_Y)
TOKEN_DEF(EDITOR_BG_ALPHA)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////
HRESULT CBSprite::loadBuffer(byte  *Buffer, bool Complete, int LifeTime, TSpriteCacheType CacheType) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(CONTINUOUS)
	TOKEN_TABLE(SPRITE)
	TOKEN_TABLE(LOOPING)
	TOKEN_TABLE(FRAME)
	TOKEN_TABLE(NAME)
	TOKEN_TABLE(PRECISE)
	TOKEN_TABLE(EDITOR_MUTED)
	TOKEN_TABLE(STREAMED_KEEP_LOADED)
	TOKEN_TABLE(STREAMED)
	TOKEN_TABLE(SCRIPT)
	TOKEN_TABLE(EDITOR_BG_FILE)
	TOKEN_TABLE(EDITOR_BG_OFFSET_X)
	TOKEN_TABLE(EDITOR_BG_OFFSET_Y)
	TOKEN_TABLE(EDITOR_BG_ALPHA)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE_END

	byte *params;
	int cmd;
	CBParser parser(Game);

	cleanup();


	if (Complete) {
		if (parser.GetCommand((char **)&Buffer, commands, (char **)&params) != TOKEN_SPRITE) {
			Game->LOG(0, "'SPRITE' keyword expected.");
			return E_FAIL;
		}
		Buffer = params;
	}

	int frame_count = 1;
	CBFrame *frame;
	while ((cmd = parser.GetCommand((char **)&Buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_CONTINUOUS:
			parser.ScanStr((char *)params, "%b", &_continuous);
			break;

		case TOKEN_EDITOR_MUTED:
			parser.ScanStr((char *)params, "%b", &_editorMuted);
			break;

		case TOKEN_SCRIPT:
			addScript((char *)params);
			break;

		case TOKEN_LOOPING:
			parser.ScanStr((char *)params, "%b", &_looping);
			break;

		case TOKEN_PRECISE:
			parser.ScanStr((char *)params, "%b", &_precise);
			break;

		case TOKEN_STREAMED:
			parser.ScanStr((char *)params, "%b", &_streamed);
			if (_streamed && LifeTime == -1) {
				LifeTime = 500;
				CacheType = CACHE_ALL;
			}
			break;

		case TOKEN_STREAMED_KEEP_LOADED:
			parser.ScanStr((char *)params, "%b", &_streamedKeepLoaded);
			break;

		case TOKEN_NAME:
			setName((char *)params);
			break;

		case TOKEN_EDITOR_BG_FILE:
			if (Game->_editorMode) {
				delete[] _editorBgFile;
				_editorBgFile = new char[strlen((char *)params) + 1];
				if (_editorBgFile) strcpy(_editorBgFile, (char *)params);
			}
			break;

		case TOKEN_EDITOR_BG_OFFSET_X:
			parser.ScanStr((char *)params, "%d", &_editorBgOffsetX);
			break;

		case TOKEN_EDITOR_BG_OFFSET_Y:
			parser.ScanStr((char *)params, "%d", &_editorBgOffsetY);
			break;

		case TOKEN_EDITOR_BG_ALPHA:
			parser.ScanStr((char *)params, "%d", &_editorBgAlpha);
			_editorBgAlpha = MIN(_editorBgAlpha, 255);
			_editorBgAlpha = MAX(_editorBgAlpha, 0);
			break;

		case TOKEN_FRAME: {
			int FrameLifeTime = LifeTime;
			if (CacheType == CACHE_HALF && frame_count % 2 != 1) FrameLifeTime = -1;

			frame = new CBFrame(Game);

			if (FAILED(frame->loadBuffer(params, FrameLifeTime, _streamedKeepLoaded))) {
				delete frame;
				Game->LOG(0, "Error parsing frame %d", frame_count);
				return E_FAIL;
			}

			_frames.Add(frame);
			frame_count++;
			if (_currentFrame == -1) _currentFrame = 0;
		}
		break;

		case TOKEN_EDITOR_PROPERTY:
			parseEditorProperty(params, false);
			break;
		}
	}

	if (cmd == PARSERR_TOKENNOTFOUND) {
		Game->LOG(0, "Syntax error in SPRITE definition");
		return E_FAIL;
	}
	_canBreak = !_continuous;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////
void CBSprite::Reset() {
	if (_frames.GetSize() > 0) _currentFrame = 0;
	else _currentFrame = -1;

	KillAllSounds();

	_lastFrameTime = 0;
	_finished = false;
	_moveX = _moveY = 0;
}


//////////////////////////////////////////////////////////////////////
bool CBSprite::GetCurrentFrame(float ZoomX, float ZoomY) {
	//if(_owner && _owner->_freezable && Game->_state == GAME_FROZEN) return true;

	if (_currentFrame == -1) return false;

	uint32 timer;
	if (_owner && _owner->_freezable) timer = Game->_timer;
	else timer = Game->_liveTimer;

	int LastFrame = _currentFrame;

	// get current frame
	if (!_paused && !_finished && timer >= _lastFrameTime + _frames[_currentFrame]->_delay && _lastFrameTime != 0) {
		if (_currentFrame < _frames.GetSize() - 1) {
			_currentFrame++;
			if (_continuous) _canBreak = (_currentFrame == _frames.GetSize() - 1);
		} else {
			if (_looping) {
				_currentFrame = 0;
				_canBreak = true;
			} else {
				_finished = true;
				_canBreak = true;
			}
		}

		_lastFrameTime = timer;
	}

	_changed = (LastFrame != _currentFrame || (_looping && _frames.GetSize() == 1));

	if (_lastFrameTime == 0) {
		_lastFrameTime = timer;
		_changed = true;
		if (_continuous) _canBreak = (_currentFrame == _frames.GetSize() - 1);
	}

	if (_changed) {
		_moveX = _frames[_currentFrame]->_moveX;
		_moveY = _frames[_currentFrame]->_moveY;

		if (ZoomX != 100 || ZoomY != 100) {
			_moveX = (int)((float)_moveX * (float)(ZoomX / 100.0f));
			_moveY = (int)((float)_moveY * (float)(ZoomY / 100.0f));
		}
	}

	return _changed;
}


//////////////////////////////////////////////////////////////////////
HRESULT CBSprite::display(int X, int Y, CBObject *Register, float ZoomX, float ZoomY, uint32 Alpha, float Rotate, TSpriteBlendMode BlendMode) {
	if (_currentFrame < 0 || _currentFrame >= _frames.GetSize()) return S_OK;

	// on change...
	if (_changed) {
		if (_frames[_currentFrame]->_killSound) {
			KillAllSounds();
		}
		applyEvent("FrameChanged");
		_frames[_currentFrame]->oneTimeDisplay(_owner, Game->_editorMode && _editorMuted);
	}

	// draw frame
	return _frames[_currentFrame]->draw(X - Game->_offsetX, Y - Game->_offsetY, Register, ZoomX, ZoomY, _precise, Alpha, _editorAllFrames, Rotate, BlendMode);
}


//////////////////////////////////////////////////////////////////////////
CBSurface *CBSprite::GetSurface() {
	// only used for animated textures for 3D models
	if (_currentFrame < 0 || _currentFrame >= _frames.GetSize()) return NULL;
	CBFrame *Frame = _frames[_currentFrame];
	if (Frame && Frame->_subframes.GetSize() > 0) {
		CBSubFrame *Subframe = Frame->_subframes[0];
		if (Subframe) return Subframe->_surface;
		else return NULL;
	} else return NULL;
}

//////////////////////////////////////////////////////////////////////////
bool CBSprite::GetBoundingRect(LPRECT Rect, int X, int Y, float ScaleX, float ScaleY) {
	if (!Rect) return false;

	CBPlatform::SetRectEmpty(Rect);
	for (int i = 0; i < _frames.GetSize(); i++) {
		RECT frame;
		RECT temp;
		CBPlatform::CopyRect(&temp, Rect);
		_frames[i]->getBoundingRect(&frame, X, Y, ScaleX, ScaleY);
		CBPlatform::UnionRect(Rect, &temp, &frame);
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBSprite::saveAsText(CBDynBuffer *Buffer, int Indent) {
	Buffer->putTextIndent(Indent, "SPRITE {\n");
	Buffer->putTextIndent(Indent + 2, "NAME=\"%s\"\n", _name);
	Buffer->putTextIndent(Indent + 2, "LOOPING=%s\n", _looping ? "TRUE" : "FALSE");
	Buffer->putTextIndent(Indent + 2, "CONTINUOUS=%s\n", _continuous ? "TRUE" : "FALSE");
	Buffer->putTextIndent(Indent + 2, "PRECISE=%s\n", _precise ? "TRUE" : "FALSE");
	if (_streamed) {
		Buffer->putTextIndent(Indent + 2, "STREAMED=%s\n", _streamed ? "TRUE" : "FALSE");

		if (_streamedKeepLoaded)
			Buffer->putTextIndent(Indent + 2, "STREAMED_KEEP_LOADED=%s\n", _streamedKeepLoaded ? "TRUE" : "FALSE");
	}

	if (_editorMuted)
		Buffer->putTextIndent(Indent + 2, "EDITOR_MUTED=%s\n", _editorMuted ? "TRUE" : "FALSE");

	if (_editorBgFile) {
		Buffer->putTextIndent(Indent + 2, "EDITOR_BG_FILE=\"%s\"\n", _editorBgFile);
		Buffer->putTextIndent(Indent + 2, "EDITOR_BG_OFFSET_X=%d\n", _editorBgOffsetX);
		Buffer->putTextIndent(Indent + 2, "EDITOR_BG_OFFSET_Y=%d\n", _editorBgOffsetY);
		Buffer->putTextIndent(Indent + 2, "EDITOR_BG_ALPHA=%d\n", _editorBgAlpha);
	}

	CBScriptHolder::saveAsText(Buffer, Indent + 2);

	int i;

	// scripts
	for (i = 0; i < _scripts.GetSize(); i++) {
		Buffer->putTextIndent(Indent + 2, "SCRIPT=\"%s\"\n", _scripts[i]->_filename);
	}


	for (i = 0; i < _frames.GetSize(); i++) {
		_frames[i]->saveAsText(Buffer, Indent + 2);
	}

	Buffer->putTextIndent(Indent, "}\n\n");

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSprite::persist(CBPersistMgr *persistMgr) {
	CBScriptHolder::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_canBreak));
	persistMgr->transfer(TMEMBER(_changed));
	persistMgr->transfer(TMEMBER(_paused));
	persistMgr->transfer(TMEMBER(_continuous));
	persistMgr->transfer(TMEMBER(_currentFrame));
	persistMgr->transfer(TMEMBER(_editorAllFrames));
	persistMgr->transfer(TMEMBER(_editorBgAlpha));
	persistMgr->transfer(TMEMBER(_editorBgFile));
	persistMgr->transfer(TMEMBER(_editorBgOffsetX));
	persistMgr->transfer(TMEMBER(_editorBgOffsetY));
	persistMgr->transfer(TMEMBER(_editorMuted));
	persistMgr->transfer(TMEMBER(_finished));

	_frames.persist(persistMgr);

	persistMgr->transfer(TMEMBER(_lastFrameTime));
	persistMgr->transfer(TMEMBER(_looping));
	persistMgr->transfer(TMEMBER(_moveX));
	persistMgr->transfer(TMEMBER(_moveY));
	persistMgr->transfer(TMEMBER(_owner));
	persistMgr->transfer(TMEMBER(_precise));
	persistMgr->transfer(TMEMBER(_streamed));
	persistMgr->transfer(TMEMBER(_streamedKeepLoaded));


	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CBSprite::scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// GetFrame
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "GetFrame") == 0) {
		stack->correctParams(1);
		int Index = stack->pop()->getInt(-1);
		if (Index < 0 || Index >= _frames.GetSize()) {
			script->RuntimeError("Sprite.GetFrame: Frame index %d is out of range.", Index);
			stack->pushNULL();
		} else stack->pushNative(_frames[Index], true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DeleteFrame
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DeleteFrame") == 0) {
		stack->correctParams(1);
		CScValue *Val = stack->pop();
		if (Val->isInt()) {
			int Index = Val->getInt(-1);
			if (Index < 0 || Index >= _frames.GetSize()) {
				script->RuntimeError("Sprite.DeleteFrame: Frame index %d is out of range.", Index);
			}
		} else {
			CBFrame *Frame = (CBFrame *)Val->getNative();
			for (int i = 0; i < _frames.GetSize(); i++) {
				if (_frames[i] == Frame) {
					if (i == _currentFrame) _lastFrameTime = 0;
					delete _frames[i];
					_frames.RemoveAt(i);
					break;
				}
			}
		}
		stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Reset
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Reset") == 0) {
		stack->correctParams(0);
		Reset();
		stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AddFrame
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AddFrame") == 0) {
		stack->correctParams(1);
		CScValue *Val = stack->pop();
		const char *Filename = NULL;
		if (!Val->isNULL()) Filename = Val->getString();

		CBFrame *Frame = new CBFrame(Game);
		if (Filename != NULL) {
			CBSubFrame *Sub = new CBSubFrame(Game);
			if (SUCCEEDED(Sub->setSurface(Filename))) {
				Sub->setDefaultRect();
				Frame->_subframes.Add(Sub);
			} else delete Sub;
		}
		_frames.Add(Frame);

		stack->pushNative(Frame, true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// InsertFrame
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "InsertFrame") == 0) {
		stack->correctParams(2);
		int Index = stack->pop()->getInt();
		if (Index < 0) Index = 0;

		CScValue *Val = stack->pop();
		const char *Filename = NULL;
		if (!Val->isNULL()) Filename = Val->getString();

		CBFrame *Frame = new CBFrame(Game);
		if (Filename != NULL) {
			CBSubFrame *Sub = new CBSubFrame(Game);
			if (SUCCEEDED(Sub->setSurface(Filename))) Frame->_subframes.Add(Sub);
			else delete Sub;
		}

		if (Index >= _frames.GetSize()) _frames.Add(Frame);
		else _frames.InsertAt(Index, Frame);

		stack->pushNative(Frame, true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Pause
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Pause") == 0) {
		stack->correctParams(0);
		_paused = true;
		stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Play
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Play") == 0) {
		stack->correctParams(0);
		_paused = false;
		stack->pushNULL();
		return S_OK;
	}

	else return CBScriptHolder::scCallMethod(script, stack, thisStack, name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CBSprite::scGetProperty(const char *name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Type") == 0) {
		_scValue->setString("sprite");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NumFrames (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "NumFrames") == 0) {
		_scValue->setInt(_frames.GetSize());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// CurrentFrame
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CurrentFrame") == 0) {
		_scValue->setInt(_currentFrame);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// PixelPerfect
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "PixelPerfect") == 0) {
		_scValue->setBool(_precise);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Looping
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Looping") == 0) {
		_scValue->setBool(_looping);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Owner (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Owner") == 0) {
		if (_owner == NULL) _scValue->setNULL();
		else _scValue->setNative(_owner, true);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Finished (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Finished") == 0) {
		_scValue->setBool(_finished);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Paused (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Paused") == 0) {
		_scValue->setBool(_paused);
		return _scValue;
	}

	else return CBScriptHolder::scGetProperty(name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSprite::scSetProperty(const char *name, CScValue *value) {
	//////////////////////////////////////////////////////////////////////////
	// CurrentFrame
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "CurrentFrame") == 0) {
		_currentFrame = value->getInt(0);
		if (_currentFrame >= _frames.GetSize() || _currentFrame < 0) {
			_currentFrame = -1;
		}
		_lastFrameTime = 0;
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PixelPerfect
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "PixelPerfect") == 0) {
		_precise = value->getBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Looping
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Looping") == 0) {
		_looping = value->getBool();
		return S_OK;
	}

	else return CBScriptHolder::scSetProperty(name, value);
}


//////////////////////////////////////////////////////////////////////////
const char *CBSprite::scToString() {
	return "[sprite]";
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSprite::KillAllSounds() {
	for (int i = 0; i < _frames.GetSize(); i++) {
		if (_frames[i]->_sound) _frames[i]->_sound->stop();
	}
	return S_OK;
}

} // end of namespace WinterMute
