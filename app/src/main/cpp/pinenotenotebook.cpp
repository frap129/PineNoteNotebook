#include <jni.h>
#include <android/log.h>
#include "displayworker.h"
#include "penworker.h"

PineNoteLib *pineNotePen = nullptr;
DisplayWorker *displayWorker = nullptr;
PenWorker *penWorker = nullptr;
JNIEnv *mEnv;

void ThrowJNIException(const char *eFile, int eLine, const char *eMessage) {
    if (eFile == nullptr || eLine == 0 || eMessage == nullptr) {
        return;
    }

    std::string eMsg = "JNIException !\n ";
    eMsg += "   File \t\t: " + std::string(eFile) + "\n";
    eMsg += "   Line \t\t: " + std::to_string(eLine) + "\n";
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

extern "C"
JNIEXPORT void JNICALL
Java_net_mulliken_pinenotenotebook_NoteJNI_nativeInit(
        JNIEnv *env, jobject thiz, jboolean use_pen, jboolean use_display) {
    ALOGD("Java_net_mulliken_pinenotenotebook_NoteJNI_nativeInit: Starting up");

    mEnv = env;

    try {
        pineNotePen = PineNoteLib::getInstance();
        displayWorker = new DisplayWorker(pineNotePen);
        penWorker = new PenWorker();

        penWorker->registerListener([](pen_event_t *event) {
            displayWorker->addPenEvent(event);
        });
    } catch (const std::exception &e) {
        ALOGE("Java_net_mulliken_pinenotenotebook_NoteJNI_nativeInit: Exception: %s", e.what());

        THROW_JAVA_EXCEPTION(e.what());
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_net_mulliken_pinenotenotebook_NoteJNI_nativeSetDisplayMode(JNIEnv *env, jobject thiz,
                                                                jint mode) {
    try {
        pineNotePen->setDisplayMode((panel_refresh_mode) mode);
    } catch (const std::exception &e) {
        ALOGE("Java_net_mulliken_pinenotenotebook_NoteJNI_nativeSetDisplayMode: Exception: %s",
              e.what());

        THROW_JAVA_EXCEPTION(e.what());
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_net_mulliken_pinenotenotebook_NoteJNI_nativeClearOverlay(JNIEnv *env, jobject thiz) {
    try {
        pineNotePen->clearOverlay();
    } catch (const std::exception &e) {
        ALOGE("Java_net_mulliken_pinenotenotebook_NoteJNI_nativeClearOverlay: Exception: %s",
              e.what());

        THROW_JAVA_EXCEPTION(e.what());
    }
}