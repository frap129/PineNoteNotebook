package net.mulliken.pinenotenotebook;

import android.content.Context;
import android.graphics.Bitmap;
import android.util.Log;
import android.view.MotionEvent;

enum EPDModes {
    EPD_AUTO,
    EPD_OVERLAY,
    EPD_FULL_GC16,
    EPD_FULL_GL16,
    EPD_FULL_GLR16,
    EPD_FULL_GLD16,
    EPD_FULL_GCC16,
    EPD_PART_GC16,
    EPD_PART_GL16,
    EPD_PART_GLR16,
    EPD_PART_GLD16,
    EPD_PART_GCC16,
    EPD_A2,
    EPD_DU,
    EPD_DU4,
    EPD_A2_ENTER,
    EPD_RESET,
}

public class NoteJNI {

    static {
        System.loadLibrary("pinenote");
    }

    private native void nativeInit(boolean usePen, boolean useDisplay);

    private native void nativeSetDisplayMode(int mode);

    private native void nativeClearOverlay();

    private native Bitmap nativeGetFullOverlayBitmap();

    private native Bitmap nativeGetOverlayBitmap();

    private static NoteJNI instance;

    public static NoteJNI getInstance() {
        if (instance == null) {
            instance = new NoteJNI();
        }

        return instance;
    }

    private Context mContext;

    private boolean isInitialized;

    private NoteJNI() {
        isInitialized = false;
    }

    public boolean isInitialized() {
        return isInitialized;
    }

    public void setDisplayMode(String mode) {
        switch (mode) {
            case "EPD_AUTO":
                nativeSetDisplayMode(EPDModes.EPD_AUTO.ordinal());
                break;
            case "EPD_OVERLAY":
                nativeSetDisplayMode(EPDModes.EPD_OVERLAY.ordinal());
                break;
            case "EPD_FULL_GC16":
                nativeSetDisplayMode(EPDModes.EPD_FULL_GC16.ordinal());
                break;
            case "EPD_FULL_GL16":
                nativeSetDisplayMode(EPDModes.EPD_FULL_GL16.ordinal());
                break;
            case "EPD_FULL_GLR16":
                nativeSetDisplayMode(EPDModes.EPD_FULL_GLR16.ordinal());
                break;
            case "EPD_FULL_GLD16":
                nativeSetDisplayMode(EPDModes.EPD_FULL_GLD16.ordinal());
                break;
            case "EPD_FULL_GCC16":
                nativeSetDisplayMode(EPDModes.EPD_FULL_GCC16.ordinal());
                break;
            case "EPD_PART_GC16":
                nativeSetDisplayMode(EPDModes.EPD_PART_GC16.ordinal());
                break;
            case "EPD_PART_GL16":
                nativeSetDisplayMode(EPDModes.EPD_PART_GL16.ordinal());
                break;
            case "EPD_PART_GLR16":
                nativeSetDisplayMode(EPDModes.EPD_PART_GLR16.ordinal());
                break;
            case "EPD_PART_GLD16":
                nativeSetDisplayMode(EPDModes.EPD_PART_GLD16.ordinal());
                break;
            case "EPD_PART_GCC16":
                nativeSetDisplayMode(EPDModes.EPD_PART_GCC16.ordinal());
                break;
            case "EPD_A2":
                nativeSetDisplayMode(EPDModes.EPD_A2.ordinal());
                break;
            case "EPD_DU":
                nativeSetDisplayMode(EPDModes.EPD_DU.ordinal());
                break;
            case "EPD_DU4":
                nativeSetDisplayMode(EPDModes.EPD_DU4.ordinal());
                break;
            case "EPD_A2_ENTER":
                nativeSetDisplayMode(EPDModes.EPD_A2_ENTER.ordinal());
                break;
            case "EPD_RESET":
                nativeSetDisplayMode(EPDModes.EPD_RESET.ordinal());
                break;
            default:
                throw new IllegalArgumentException("Unknown display mode: " + mode);
        }
    }

    public void clearDisplay() {
        nativeClearOverlay();
    }

    public void init(Context context) {
        mContext = context;

        nativeInit(true, true);
        isInitialized = true;
    }
}
