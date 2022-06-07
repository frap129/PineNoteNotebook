package net.mulliken.pinenotenotebook;

import android.content.Context;
import android.content.DialogInterface;
import android.graphics.Bitmap;
import android.graphics.Rect;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.os.Handler;
import android.util.AttributeSet;
import android.util.DisplayMetrics;
import android.view.View;
import android.widget.ImageView;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AlertDialog;

public class NoteView extends View {
    private static final String TAG = "NoteView";
    private static int displayHeight = 0;

    private Rect windowRect = new Rect();

    static {
        System.loadLibrary("pinenote");
    }


    private native Bitmap nativeGetBitmap();
    private native void nativeOnAttachedToWindow();
    private native void nativeOnDetachedFromWindow();
    private native void nativeOnSizeChanged(int left, int top, int right, int bottom);
    private native void nativeOnWindowFocusChanged(boolean hasFocus);

    public NoteView(Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
    }

    public Bitmap getInputAsBitmap() {
        return nativeGetBitmap();
    }


    @Override
    protected void onAttachedToWindow() {
        super.onAttachedToWindow();

        // Get screen dimensions
        DisplayMetrics displayMetrics = new DisplayMetrics();
        getDisplay().getMetrics(displayMetrics);
        displayHeight = displayMetrics.heightPixels;

        nativeOnAttachedToWindow();

        Handler handler = new Handler();
        handler.postDelayed(new Runnable() {
            @Override
            public void run() {
                ImageView imageView = new ImageView(getContext());
                Drawable d = new BitmapDrawable(getResources(), nativeGetBitmap());
                imageView.setImageDrawable(d);

                AlertDialog dialog = new AlertDialog.Builder(getContext())
                        .setView(imageView)
                        .setPositiveButton("ok", new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                dialog.dismiss();
                            }
                        }).create();

                dialog.show();
            }
        }, 20000);
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

        windowRect.left = left;
        windowRect.top = top;
        windowRect.right = right;
        windowRect.bottom = bottom;

        nativeOnSizeChanged(left, top, right, bottom);
    }

    @Override
    public void onWindowFocusChanged(boolean hasWindowFocus) {
        super.onWindowFocusChanged(hasWindowFocus);

        nativeOnWindowFocusChanged(hasWindowFocus);

        if (hasWindowFocus) {
            nativeOnSizeChanged(
                    windowRect.left, windowRect.top, windowRect.right, windowRect.bottom);
        }
    }

    @Override
    protected void onDraw(android.graphics.Canvas canvas) {
        super.onDraw(canvas);

        // TODO: draw the note
    }
}
