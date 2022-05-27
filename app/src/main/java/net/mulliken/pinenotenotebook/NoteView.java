package net.mulliken.pinenotenotebook;

import android.content.Context;
import android.content.res.Configuration;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;

import androidx.annotation.Nullable;

public class NoteView extends View {

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
        int orientation = getResources().getConfiguration().orientation;

        int left = getLeft();
        int top = getTop();
        int right = getRight();
        int bottom = getBottom();

        Log.d("NoteView", "onSizeChanged:");
        Log.d("NoteView", "  orientation: " + orientation);
        Log.d("NoteView", "  left: " + left);
        Log.d("NoteView", "  top: " + top);
        Log.d("NoteView", "  right: " + right);
        Log.d("NoteView", "  bottom: " + bottom);

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
