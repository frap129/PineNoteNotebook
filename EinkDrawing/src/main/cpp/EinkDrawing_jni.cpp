#include <jni.h>
#include <android/log.h>
#include <android/bitmap.h>
#include "pinenotelib.h"
#include "displayworker.h"
#include "penworker.h"

PineNoteLib *pineNotePen = nullptr;
PenWorker *penWorker = nullptr;
DisplayWorker *displayWorker = nullptr;
InputState* mState = nullptr;

JNIEnv *mEnv;

void ThrowJNIException(const char *eFile, int eLine, const char *eMessage) {
    if (eFile == nullptr || eLine == 0 || eMessage == nullptr) {
        return;
    }

    std::string eMsg = "JNIException !\n ";
    eMsg += "   File \t\t: " + std::string(eFile) + "\n";
    eMsg += "   LineSegment \t\t: " + std::to_string(eLine) + "\n";
    eMsg += "   Message \t: " + std::string(eMessage) + "\n";

    jclass tClass = mEnv->FindClass("java/lang/Exception");
    if (tClass == nullptr) {
        return;
    }

    mEnv->ThrowNew(tClass, eMsg.c_str());
    mEnv->DeleteLocalRef(tClass);
}

#define THROW_JAVA_EXCEPTION(_INFO_) \
    ThrowJNIException(__FILE__, __LINE__, _INFO_)

void startPen() {
    pineNotePen = PineNoteLib::getInstance();
    pineNotePen->enableOverlay();

    mState = static_cast<InputState *>(malloc(sizeof(InputState)));
    mState = new InputState;

    if (displayWorker == nullptr) {
        displayWorker = new DisplayWorker(mState);
    }
}

void enablePen() {
    if (displayWorker != nullptr) {
        displayWorker->startWorker();
    }

    if (penWorker == nullptr) {
        penWorker = new PenWorker();

        // We will assume that if it is not null, it is already initialized.
        penWorker->registerListener([](pen_event_t event) {
            displayWorker->onPenEvent(event);
        });
    }
}

void disablePen() {
    if (mState != nullptr) {
        uint32_t *pixelData = pineNotePen->getFullPixelData();
        mState->prevPixelData = *pixelData;
        free(pixelData);
    }

    if (penWorker != nullptr) {
        delete penWorker;
        penWorker = nullptr;
    }

    if (displayWorker != nullptr) {
        pen_event_t event{};
        event.action = EXIT_WORKER;
        displayWorker->onPenEvent(event);
    }
}

void stopPen() {
    disablePen();

    if (displayWorker != nullptr) {
        pen_event_t event{};
        event.action = EXIT_WORKER;
        displayWorker->onPenEvent(event);

        delete displayWorker;
        displayWorker = nullptr;
    }

    pineNotePen->disableOverlay();
    PineNoteLib::destroyInstance();

    free(mState);
}

extern "C"
JNIEXPORT void JNICALL
Java_net_mulliken_einkdrawing_NoteView_nativeOnAttachedToWindow(JNIEnv *env, jobject thiz
) {
    ALOGD("Java_net_mulliken_einkdrawing_NoteView_nativeOnAttachedToWindow: Attaching to window");
    mEnv = env;
    startPen();
}

extern "C"
JNIEXPORT void JNICALL
Java_net_mulliken_einkdrawing_NoteView_nativeOnDetachedFromWindow(JNIEnv *env, jobject thiz) {
    ALOGD("Java_net_mulliken_einkdrawing_NoteView_nativeOnDetachedFromWindow: Detaching from window");
    mEnv = env;

    stopPen();
}

extern "C"
JNIEXPORT void JNICALL
Java_net_mulliken_einkdrawing_NoteView_nativeOnSizeChanged(JNIEnv *env, jobject thiz,
                                                                jint left, jint top, jint right,
                                                                jint bottom) {
    if (pineNotePen == nullptr) {
        pineNotePen = PineNoteLib::getInstance();
    }

    // Invert x and y so the display is correct.
    pineNotePen->setDrawArea(bottom, left, top, right);
}

extern "C"
JNIEXPORT void JNICALL
Java_net_mulliken_einkdrawing_NoteView_nativeOnWindowFocusChanged(JNIEnv *env, jobject thiz,
                                                                       jboolean has_focus) {
    if (has_focus) {
        ALOGD("Java_net_mulliken_einkdrawing_NoteView_nativeOnWindowFocusChanged: Focus gained");
        enablePen();
    } else {
        ALOGD("Java_net_mulliken_einkdrawing_NoteView_nativeOnWindowFocusChanged: Focus lost");
        disablePen();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_net_mulliken_einkdrawing_NoteView_nativeStartPen(JNIEnv *env, jobject thiz) {
    startPen();
}

extern "C"
JNIEXPORT void JNICALL
Java_net_mulliken_einkdrawing_NoteView_nativeStopPen(JNIEnv *env, jobject thiz) {
    stopPen();
}

jobject createBitmapFromPixelData(JNIEnv *env, int _width, int _height, const uint32_t* pixelData) {
    jclass bitmapConfig = env->FindClass("android/graphics/Bitmap$Config");
    jfieldID rgba8888FieldID = env->GetStaticFieldID(bitmapConfig, "ARGB_8888", "Landroid/graphics/Bitmap$Config;");
    jobject rgba8888Obj = env->GetStaticObjectField(bitmapConfig, rgba8888FieldID);

    jclass bitmapClass = env->FindClass("android/graphics/Bitmap");
    jmethodID createBitmapMethodID = env->GetStaticMethodID(bitmapClass, "createBitmap", "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
    jobject bitmapObj = env->CallStaticObjectMethod(bitmapClass, createBitmapMethodID, _width, _height, rgba8888Obj);

    jintArray pixels = env->NewIntArray(_width * _height);
    for (int i = 0; i < _width * _height; i++)
    {
        int currentPixel = pixelData[i];
        env->SetIntArrayRegion(pixels, i, 1, &currentPixel);
    }

    jmethodID setPixelsMid = env->GetMethodID(bitmapClass, "setPixels", "([IIIIIII)V");
    env->CallVoidMethod(bitmapObj, setPixelsMid, pixels, 0, _width, 0, 0, _width, _height);
    return bitmapObj;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_net_mulliken_einkdrawing_NoteView_nativeGetFullOverlayBitmap(JNIEnv *env, jobject obj)
{
    uint32_t *pixelData = pineNotePen->getFullPixelData();
    int _width = pineNotePen->ebc_info.width;
    int _height = pineNotePen->ebc_info.height;

    jobject bitmap = createBitmapFromPixelData(env, _width, _height, pixelData);
    free(pixelData);

    return bitmap;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_net_mulliken_einkdrawing_NoteView_nativeGetOverlayBitmap(JNIEnv *env, jobject obj)
{
    uint32_t *pixelData = pineNotePen->getBoundedPixelData();
    int _width = pineNotePen->display_y2 - pineNotePen->display_y1;
    int _height = pineNotePen->display_x2 - pineNotePen->display_x1;

    jobject bitmap = createBitmapFromPixelData(env, _width, _height, pixelData);
    free(pixelData);

    return bitmap;
}

extern "C"
JNIEXPORT void JNICALL
Java_net_mulliken_einkdrawing_NoteView_nativeSetColor(JNIEnv *env, jobject obj, jint color) {
    if (mState != nullptr) {
        mState->inputColor = color;
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_net_mulliken_einkdrawing_NoteView_nativeSetWidth(JNIEnv *env, jobject obj, jint width) {
    if (mState != nullptr) {
        mState->inputWidth = width;
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_net_mulliken_einkdrawing_NoteView_nativeSetOverlayBitmap(JNIEnv * env,
                                    jobject obj, jobject bitmap)
{
    AndroidBitmapInfo androidBitmapInfo ;
    void* pixels;
    AndroidBitmap_getInfo(env, bitmap, &androidBitmapInfo);
    AndroidBitmap_lockPixels(env, bitmap, &pixels);
    auto* pixelData = (uint32_t *) pixels;

    if (pineNotePen != nullptr) {
        pineNotePen->setBoundedPixelData(pixelData);
    }
    AndroidBitmap_unlockPixels(env, bitmap);
}

extern "C"
JNIEXPORT void JNICALL
Java_net_mulliken_einkdrawing_NoteView_nativeSetFullOverlayBitmap(JNIEnv * env,
                                    jobject obj, jobject bitmap)
{
    AndroidBitmapInfo androidBitmapInfo ;
    void* pixels;
    AndroidBitmap_getInfo(env, bitmap, &androidBitmapInfo);
    AndroidBitmap_lockPixels(env, bitmap, &pixels);
    auto* pixelData = (uint32_t *) pixels;

    if (pineNotePen != nullptr) {
        pineNotePen->setFullPixelData(pixelData);
    }
    AndroidBitmap_unlockPixels(env, bitmap);
}
