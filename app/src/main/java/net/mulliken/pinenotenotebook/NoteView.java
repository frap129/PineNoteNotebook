package net.mulliken.pinenotenotebook;

import android.content.Context;
import android.util.AttributeSet;
import android.view.View;

import androidx.annotation.Nullable;

public class NoteView extends View {
    private NoteJNI mNative;

    public NoteView(Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);

        mNative = NoteJNI.getInstance();
    }

    @Override
    protected void onAttachedToWindow() {
        super.onAttachedToWindow();

        mNative.init(getContext());
    }

    @Override
    protected void onDetachedFromWindow() {
        super.onDetachedFromWindow();
    }

    @Override
    protected void onDraw(android.graphics.Canvas canvas) {
        super.onDraw(canvas);

        // TODO: draw the note
    }
}
