package org.project_cabal.cabal;

import android.content.Context;
import android.view.MotionEvent;
import android.view.InputDevice;

public class CabalEventsHoneycomb extends CabalEvents {

	public CabalEventsHoneycomb(Context context, Cabal cabal, MouseHelper mouseHelper) {
		super(context, cabal, mouseHelper);
	}

	@Override
	public boolean onGenericMotionEvent(MotionEvent e) {
		if((e.getSource() & InputDevice.SOURCE_CLASS_JOYSTICK) != 0) {
			_cabal.pushEvent(JE_JOYSTICK, e.getAction(),
					   (int)(e.getAxisValue(MotionEvent.AXIS_X)*100),
					   (int)(e.getAxisValue(MotionEvent.AXIS_Y)*100),
					   0, 0);
			return true;
		}

		return false;
	}
}
