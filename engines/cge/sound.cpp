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
 * This code is based on original Soltys source code
 * Copyright (c) 1994-1995 Janus B. Wisniewski and L.K. Avalon
 */

#include "cge/general.h"
#include "cge/startup.h"
#include "cge/sound.h"
#include "cge/text.h"
#include "cge/cfile.h"
#include "cge/vol.h"


namespace CGE {

bool  _music  = true;
FX    _fx     = 16;   // must precede SOUND!!
SOUND _sound;


SOUND::SOUND(void) {
	if (Startup::_soundOk)
		Open();
}


SOUND::~SOUND(void) {
	Close();
}


void SOUND::Close(void) {
	KillMIDI();
	sndDone();
}


void SOUND::Open(void) {
	sndInit();
	Play(_fx[30000], 8);
}


void SOUND::Play(DATACK *wav, int pan, int cnt) {
	if (wav) {
		Stop();
		smpinf._saddr = (uint8 *) &*(wav->EAddr());
		smpinf._slen = (uint16)wav->Size();
		smpinf._span = pan;
		smpinf._sflag = cnt;
		sndDigiStart(&smpinf);
	}
}


void SOUND::Stop(void) {
	sndDigiStop(&smpinf);
}


FX::FX(int size) : Emm(0L), Current(NULL) {
	Cache = new HAN[size];
	for (Size = 0; Size < size; Size++) {
		Cache[Size].Ref = 0;
		Cache[Size].Wav = NULL;
	}
}


FX::~FX(void) {
	Clear();
	delete[] Cache;
}


void FX::Clear(void) {
	HAN *p, * q;
	for (p = Cache, q = p + Size; p < q; p++) {
		if (p->Ref) {
			p->Ref = 0;
			delete p->Wav;
			p->Wav = NULL;
		}
	}
	Emm.release();
	Current = NULL;
}


int FX::Find(int ref) {
	HAN *p, * q;
	int i = 0;
	for (p = Cache, q = p + Size; p < q; p++) {
		if (p->Ref == ref)
			break;
		else
			++i;
	}
	return i;
}


void FX::Preload(int ref0) {
	HAN *CacheLim = Cache + Size;
	int ref;

	for (ref = ref0; ref < ref0 + 10; ref++) {
		static char fname[] = "FX00000.WAV";
		wtom(ref, fname + 2, 10, 5);
		INI_FILE file = INI_FILE(fname);
		DATACK *wav = LoadWave(&file, &Emm);
		if (wav) {
			HAN *p = &Cache[Find(0)];
			if (p >= CacheLim)
				break;
			p->Wav = wav;
			p->Ref = ref;
		}
	}
}


DATACK *FX::Load(int idx, int ref) {
	static char fname[] = "FX00000.WAV";
	wtom(ref, fname + 2, 10, 5);

	INI_FILE file = INI_FILE(fname);
	DATACK *wav = LoadWave(&file, &Emm);
	if (wav) {
		HAN *p = &Cache[idx];
		p->Wav = wav;
		p->Ref = ref;
	}
	return wav;
}


DATACK *FX::operator [](int ref) {
	int i;
	if ((i = Find(ref)) < Size)
		Current = Cache[i].Wav;
	else {
		if ((i = Find(0)) >= Size) {
			Clear();
			i = 0;
		}
		Current = Load(i, ref);
	}
	return Current;
}


static  uint8  *midi    = NULL;


void KillMIDI(void) {
	sndMidiStop();
	if (midi) {
		delete[] midi;
		midi = NULL;
	}
}


void LoadMIDI(int ref) {
	static char fn[] = "00.MID";
	wtom(ref, fn, 10, 2);
	if (INI_FILE::exist(fn)) {
		KillMIDI();
		INI_FILE mid = fn;
		if (mid._error == 0) {
			uint16 siz = (uint16) mid.size();
			midi = new uint8[siz];
			if (midi) {
				mid.read(midi, siz);
				if (mid._error)
					KillMIDI();
				else
					sndMidiStart(midi);
			}
		}
	}
}


void *Patch(int pat) {
	void *p = NULL;
	static char fn[] = "PATCH000.SND";

	wtom(pat, fn + 5, 10, 3);
	INI_FILE snd = fn;
	if (!snd._error) {
		uint16 siz = (uint16) snd.size();
		p = (uint8 *) malloc(siz);
		if (p) {
			snd.read(p, siz);
			if (snd._error) {
				free(p);
				p = NULL;
			}
		}
	}
	return p;
}

} // End of namespace CGE
