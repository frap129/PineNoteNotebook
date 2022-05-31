package net.mulliken.pinenotenotebook;

import android.content.Context;
import android.content.res.Configuration;
import android.util.AttributeSet;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.View;

import androidx.annotation.Nullable;

public class NoteView extends View {
    private static final String TAG = "NoteView";
    private static int displayHeight = 0;
    private static int displayWidth = 0;

    static {
        System.loadLibrary("pinenote");
    }

    private native void nativeOnAttachedToWindow();
    private native void nativeOnDetachedFromWindow();
    private native void nativeOnSizeChanged(int left, int top, int right, int bottom);
    private native void nativeOnWindowFocusChanged(boolean hasFocus);

    public NoteView(Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    protected void onAttachedToWindow() {
        super.onAttachedToWindow();

        // Get screen dimensions
        DisplayMetrics displayMetrics = new DisplayMetrics();
        getDisplay().getMetrics(displayMetrics);
        displayHeight = displayMetrics.heightPixels;
        displayWidth = displayMetrics.widthPixels;

        nativeOnAttachedToWindow();
    }

    @Override
    protected void onDetachedFromWindow() {
        super.onDetachedFromWindow();

        nativeOnDetachedFromWindow();
    }

    @Override
    protected void onSizeChanged(int w, int h, int old_width, int old_height) {
        super.onSizeChanged(w, h, old_width, old_height);

        // Get the orientation of the device
        int[] l = new int[2];
        getLocationOnScreen(l);
        int left = l[0];
        int right = left + w;

        // Android considers the top of the screen as 0, so we need to invert the y coordinates
        int bottom = displayHeight - h - l[1];
        int top = displayHeight - l[1];

        Log.d(TAG, "onSizeChanged:");
        Log.d(TAG, "  left: " + left);
        Log.d(TAG, "  top: " + top);
        Log.d(TAG, "  right: " + right);
        Log.d(TAG, "  bottom: " + bottom);

        nativeOnSizeChanged(left, top, right, bottom);
    }

    @Override
    public void onWindowFocusChanged(boolean hasWindowFocus) {
        super.onWindowFocusChanged(hasWindowFocus);

        nativeOnWindowFocusChanged(hasWindowFocus);
    }

    @Override
    protected void onDraw(android.graphics.Canvas canvas) {
        super.onDraw(canvas);

        // TODO: draw the note
    }
}
