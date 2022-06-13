package net.mulliken.einkdrawing

import android.content.Context
import android.graphics.Bitmap
import android.graphics.Canvas
import android.graphics.Rect
import android.util.AttributeSet

import android.util.DisplayMetrics
import android.view.View


class NoteView(context: Context?, attrs: AttributeSet?) : View(context, attrs) {
    companion object {
        init {
            System.loadLibrary("EinkDrawing")
        }
    }

    private val TAG = "NoteView"
    private var displayHeight = 0

    private val windowRect: Rect = Rect()

    private external fun nativeOnAttachedToWindow()
    private external fun nativeOnDetachedFromWindow()
    private external fun nativeOnSizeChanged(left: Int, top: Int, right: Int, bottom: Int)
    private external fun nativeOnWindowFocusChanged(hasFocus: Boolean)
    private external fun nativeGetFullOverlayBitmap(): Bitmap?
    private external fun nativeGetOverlayBitmap(): Bitmap?

    override fun onAttachedToWindow() {
        super.onAttachedToWindow()

        // Get screen dimensions
        val displayMetrics = DisplayMetrics()
        getDisplay().getMetrics(displayMetrics)
        displayHeight = displayMetrics.heightPixels
        nativeOnAttachedToWindow()
    }

    override fun onDetachedFromWindow() {
        super.onDetachedFromWindow()
        nativeOnDetachedFromWindow()
    }

    override fun onSizeChanged(w: Int, h: Int, old_width: Int, old_height: Int) {
        super.onSizeChanged(w, h, old_width, old_height)

        // Get the orientation of the device
        val l = IntArray(2)
        getLocationOnScreen(l)
        val left = l[0]
        val right = left + w

        // Android considers the top of the screen as 0, so we need to invert the y coordinates
        val bottom = displayHeight - h - l[1]
        val top = displayHeight - l[1]
        windowRect.left = left
        windowRect.top = top
        windowRect.right = right
        windowRect.bottom = bottom
        nativeOnSizeChanged(left, top, right, bottom)
    }

    override fun onWindowFocusChanged(hasWindowFocus: Boolean) {
        super.onWindowFocusChanged(hasWindowFocus)
        nativeOnWindowFocusChanged(hasWindowFocus)
        if (hasWindowFocus) {
            nativeOnSizeChanged(
                windowRect.left, windowRect.top, windowRect.right, windowRect.bottom
            )
        }
    }

    override fun onDraw(canvas: Canvas?) {
        super.onDraw(canvas)

        // TODO: draw the note
    }

    fun getFullOverlayBitmap(): Bitmap? = nativeGetFullOverlayBitmap()

    fun getOverlayBitmap(): Bitmap? = nativeGetOverlayBitmap()
}