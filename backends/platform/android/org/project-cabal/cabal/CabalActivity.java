package org.project_cabal.cabal;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.media.AudioManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.SurfaceView;
import android.view.SurfaceHolder;
import android.view.MotionEvent;
import android.view.inputmethod.InputMethodManager;
import android.widget.Toast;

import java.io.File;

public class CabalActivity extends Activity {

	/* Establish whether the hover events are available */
	private static boolean _hoverAvailable;

	static {
		try {
			MouseHelper.checkHoverAvailable(); // this throws exception if we're on too old version
			_hoverAvailable = true;
		} catch (Throwable t) {
			_hoverAvailable = false;
		}
	}

	private class MyCabal extends Cabal {
		private boolean usingSmallScreen() {
			// Multiple screen sizes came in with Android 1.6.  Have
			// to use reflection in order to continue supporting 1.5
			// devices :(
			DisplayMetrics metrics = new DisplayMetrics();
			getWindowManager().getDefaultDisplay().getMetrics(metrics);

			try {
				// This 'density' term is very confusing.
				int DENSITY_LOW = metrics.getClass().getField("DENSITY_LOW").getInt(null);
				int densityDpi = metrics.getClass().getField("densityDpi").getInt(metrics);
				return densityDpi <= DENSITY_LOW;
			} catch (Exception e) {
				return false;
			}
		}

		public MyCabal(SurfaceHolder holder) {
			super(CabalActivity.this.getAssets(), holder);

			// Enable zoning on 'small' screens.
			// FIXME make this optional for the user
			// disabled for now since it crops too much
			//enableZoning(usingSmallScreen());
		}

		@Override
		protected void getDPI(float[] values) {
			DisplayMetrics metrics = new DisplayMetrics();
			getWindowManager().getDefaultDisplay().getMetrics(metrics);

			values[0] = metrics.xdpi;
			values[1] = metrics.ydpi;
		}

		@Override
		protected void displayMessageOnOSD(String msg) {
			Log.i(LOG_TAG, "OSD: " + msg);
			Toast.makeText(CabalActivity.this, msg, Toast.LENGTH_LONG).show();
		}

		@Override
		protected void setWindowCaption(final String caption) {
			runOnUiThread(new Runnable() {
					public void run() {
						setTitle(caption);
					}
				});
		}

		@Override
		protected void showVirtualKeyboard(final boolean enable) {
			runOnUiThread(new Runnable() {
					public void run() {
						showKeyboard(enable);
					}
				});
		}

		@Override
		protected String[] getSysArchives() {
			return new String[0];
		}

	}

	private MyCabal _cabal;
	private CabalEvents _events;
	private MouseHelper _mouseHelper;
	private Thread _cabal_thread;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		setVolumeControlStream(AudioManager.STREAM_MUSIC);

		setContentView(R.layout.main);
		takeKeyEvents(true);

		// This is a common enough error that we should warn about it
		// explicitly.
		if (!Environment.getExternalStorageDirectory().canRead()) {
			new AlertDialog.Builder(this)
				.setTitle(R.string.no_sdcard_title)
				.setIcon(android.R.drawable.ic_dialog_alert)
				.setMessage(R.string.no_sdcard)
				.setNegativeButton(R.string.quit,
									new DialogInterface.OnClickListener() {
										public void onClick(DialogInterface dialog,
															int which) {
											finish();
										}
									})
				.show();

			return;
		}

		SurfaceView main_surface = (SurfaceView)findViewById(R.id.main_surface);

		main_surface.requestFocus();

		getFilesDir().mkdirs();

		// Store savegames on external storage if we can, which means they're
		// world-readable and don't get deleted on uninstall.
		String savePath = Environment.getExternalStorageDirectory() + "/Cabal/Saves/";
		File saveDir = new File(savePath);
		saveDir.mkdirs();
		if (!saveDir.isDirectory()) {
			// If it doesn't work, resort to the internal app path.
			savePath = getDir("saves", MODE_WORLD_READABLE).getPath();
		}

		// Start Cabal
		_cabal = new MyCabal(main_surface.getHolder());

		_cabal.setArgs(new String[] {
			"cabalexec",
			"--config=" + getFileStreamPath("cabalexecrc").getPath(),
			"--path=" + Environment.getExternalStorageDirectory().getPath(),
			"--savepath=" + savePath
		});

		Log.d(Cabal.LOG_TAG, "Hover available: " + _hoverAvailable);
		if (_hoverAvailable) {
			_mouseHelper = new MouseHelper(_cabal);
			_mouseHelper.attach(main_surface);
		}

		if (Build.VERSION.SDK_INT < Build.VERSION_CODES.HONEYCOMB_MR1)
		{
			_events = new CabalEvents(this, _cabal, _mouseHelper);
		}
		else
		{
			_events = new CabalEventsHoneycomb(this, _cabal, _mouseHelper);
		}

		main_surface.setOnKeyListener(_events);
		main_surface.setOnTouchListener(_events);

		_cabal_thread = new Thread(_cabal, "Cabal");
		_cabal_thread.start();
	}

	@Override
	public void onStart() {
		Log.d(Cabal.LOG_TAG, "onStart");

		super.onStart();
	}

	@Override
	public void onResume() {
		Log.d(Cabal.LOG_TAG, "onResume");

		super.onResume();

		if (_cabal != null)
			_cabal.setPause(false);
		showMouseCursor(false);
	}

	@Override
	public void onPause() {
		Log.d(Cabal.LOG_TAG, "onPause");

		super.onPause();

		if (_cabal != null)
			_cabal.setPause(true);
		showMouseCursor(true);
	}

	@Override
	public void onStop() {
		Log.d(Cabal.LOG_TAG, "onStop");

		super.onStop();
	}

	@Override
	public void onDestroy() {
		Log.d(Cabal.LOG_TAG, "onDestroy");

		super.onDestroy();

		if (_events != null) {
			_events.sendQuitEvent();

			try {
				// 1s timeout
				_cabal_thread.join(1000);
			} catch (InterruptedException e) {
				Log.i(Cabal.LOG_TAG, "Error while joining Cabal thread", e);
			}

			_cabal = null;
		}
	}

	@Override
	public boolean onTrackballEvent(MotionEvent e) {
		if (_events != null)
			return _events.onTrackballEvent(e);

		return false;
	}

	@Override
	public boolean onGenericMotionEvent(final MotionEvent e) {
		if (_events != null)
			return _events.onGenericMotionEvent(e);

		return false;
	}

	private void showKeyboard(boolean show) {
		SurfaceView main_surface = (SurfaceView)findViewById(R.id.main_surface);
		InputMethodManager imm = (InputMethodManager)
			getSystemService(INPUT_METHOD_SERVICE);

		if (show)
			imm.showSoftInput(main_surface, InputMethodManager.SHOW_IMPLICIT);
		else
			imm.hideSoftInputFromWindow(main_surface.getWindowToken(),
										InputMethodManager.HIDE_IMPLICIT_ONLY);
	}

	private void showMouseCursor(boolean show) {
		/* Currently hiding the system mouse cursor is only
		   supported on OUYA.  If other systems provide similar
		   intents, please add them here as well */
		Intent intent =
			new Intent(show?
				   "tv.ouya.controller.action.SHOW_CURSOR" :
				   "tv.ouya.controller.action.HIDE_CURSOR");
		sendBroadcast(intent);
	}
}
