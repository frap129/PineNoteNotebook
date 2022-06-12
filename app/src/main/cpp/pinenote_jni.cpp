#include <jni.h>
#include <stdio.h>
#include <android/bitmap.h>
#include <cstring>
#include <unistd.h>
#include <android/log.h>
#include "pinenotelib.h"
#include "displayworker.h"
#include "penworker.h"

PineNoteLib *pineNotePen = nullptr;
PenWorker *penWorker = nullptr;
DisplayWorker *displayWorker = nullptr;

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

void enablePen() {
    pineNotePen = PineNoteLib::getInstance();
    pineNotePen->enableOverlay();

    if (penWorker == nullptr) {
        penWorker = new PenWorker();
    }

    if (displayWorker == nullptr) {
        displayWorker = new DisplayWorker();

        // We will assume that if it is not null, it is already initialized.
        penWorker->registerListener([](pen_event_t event) {
            displayWorker->onPenEvent(event);
        });
    }
}

void disablePen() {
    if (displayWorker != nullptr) {
        pen_event_t event{};
        event.action = EXIT_WORKER;
        displayWorker->onPenEvent(event);

        delete displayWorker;
        displayWorker = nullptr;
    }

    if (penWorker != nullptr) {
        delete penWorker;
        penWorker = nullptr;
    }

    pineNotePen->disableOverlay();
    PineNoteLib::destroyInstance();
}

extern "C"
JNIEXPORT void JNICALL
Java_net_mulliken_pinenotenotebook_NoteView_nativeOnAttachedToWindow(JNIEnv *env, jobject thiz
) {
    ALOGD("Java_net_mulliken_pinenotenotebook_NoteView_nativeOnAttachedToWindow: Attaching to window");
    mEnv = env;
}

extern "C"
JNIEXPORT void JNICALL
Java_net_mulliken_pinenotenotebook_NoteView_nativeOnDetachedFromWindow(JNIEnv *env, jobject thiz) {
    ALOGD("Java_net_mulliken_pinenotenotebook_NoteView_nativeOnDetachedFromWindow: Detaching from window");
    mEnv = env;

    disablePen();
}

extern "C"
JNIEXPORT void JNICALL
Java_net_mulliken_pinenotenotebook_NoteView_nativeOnSizeChanged(JNIEnv *env, jobject thiz,
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
Java_net_mulliken_pinenotenotebook_NoteView_nativeOnWindowFocusChanged(JNIEnv *env, jobject thiz,
                                                                       jboolean has_focus) {
    if (has_focus) {
        ALOGD("Java_net_mulliken_pinenotenotebook_NoteView_nativeOnWindowFocusChanged: Focus gained");
        enablePen();
    } else {
        ALOGD("Java_net_mulliken_pinenotenotebook_NoteView_nativeOnWindowFocusChanged: Focus lost");
        disablePen();
    }
}

/**restore java bitmap (from JNI data)*/ //
extern "C"
JNIEXPORT jobject JNICALL
Java_net_mulliken_pinenotenotebook_NoteView_nativeGetBitmap(JNIEnv * jniEnv, jobject obj)
{
    uint32_t *pixelBuffer = pineNotePen->getPixelData();
    int _width = pineNotePen->ebc_info.width;
    int _height = pineNotePen->ebc_info.height;

    jclass bitmapConfig = jniEnv->FindClass("android/graphics/Bitmap$Config");
    jfieldID rgba8888FieldID = jniEnv->GetStaticFieldID(bitmapConfig, "ARGB_8888", "Landroid/graphics/Bitmap$Config;");
    jobject rgba8888Obj = jniEnv->GetStaticObjectField(bitmapConfig, rgba8888FieldID);

    jclass bitmapClass = jniEnv->FindClass("android/graphics/Bitmap");
    jmethodID createBitmapMethodID = jniEnv->GetStaticMethodID(bitmapClass,"createBitmap", "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
    jobject bitmapObj = jniEnv->CallStaticObjectMethod(bitmapClass, createBitmapMethodID, _width, _height, rgba8888Obj);

    jintArray pixels = jniEnv->NewIntArray(_width * _height);
    for (int i = 0; i < _width * _height; i++)
    {
        int currentPixel = pixelBuffer[i];
        jniEnv->SetIntArrayRegion(pixels, i, 1, &currentPixel);
    }

    jmethodID setPixelsMid = jniEnv->GetMethodID(bitmapClass, "setPixels", "([IIIIIII)V");
    jniEnv->CallVoidMethod(bitmapObj, setPixelsMid, pixels, 0, _width, 0, 0, _width, _height);
    return bitmapObj;
}