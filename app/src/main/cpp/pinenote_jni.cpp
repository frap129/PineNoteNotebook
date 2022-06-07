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
Java_net_mulliken_pinenotenotebook_NoteView_nativeGetBitmap(JNIEnv * env, jobject obj, jobject handle)
{
    BitmapImage bitmap = pineNotePen->getBitmap();
    //creating a new bitmap to put the pixels into it - using Bitmap Bitmap.createBitmap (int width, int height, Bitmap.Config config) :
    //
    jclass bitmapCls = env->FindClass("android/graphics/Bitmap");
    jmethodID createBitmapFunction = env->GetStaticMethodID(bitmapCls,
                                                            "createBitmap",
                                                            "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
    jstring configName = env->NewStringUTF("ARGB_8888");
    jclass bitmapConfigClass = env->FindClass("android/graphics/Bitmap$Config");
    jmethodID valueOfBitmapConfigFunction = env->GetStaticMethodID(
            bitmapConfigClass, "valueOf",
            "(Ljava/lang/String;)Landroid/graphics/Bitmap$Config;");
    jobject bitmapConfig = env->CallStaticObjectMethod(bitmapConfigClass,
                                                       valueOfBitmapConfigFunction, configName);
    jobject newBitmap = env->CallStaticObjectMethod(bitmapCls,
                                                    createBitmapFunction, bitmap.infoHeader.width,
                                                    bitmap.infoHeader.width, bitmapConfig);
    //
    // putting the pixels into the new bitmap:
    //
    int ret;
    void* bitmapPixels;
    if ((ret = AndroidBitmap_lockPixels(env, newBitmap, &bitmapPixels)) < 0)
    {
        ALOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
        return NULL;
    }
    uint32_t* newBitmapPixels = (uint32_t*) bitmapPixels;
    memcpy(newBitmapPixels, bitmap.bitmapBuffer,
           bitmap.bitmapBufferSize);
    AndroidBitmap_unlockPixels(env, newBitmap);
    //LOGD("returning the new bitmap");
    return newBitmap;
}