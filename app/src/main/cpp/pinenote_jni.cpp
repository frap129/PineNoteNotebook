#include <jni.h>
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

void enablePen() {
    pineNotePen = PineNoteLib::getInstance();
    pineNotePen->enableOverlay();

    if (penWorker == nullptr) {
        penWorker = new PenWorker();
    }

    if (displayWorker == nullptr) {
        displayWorker = new DisplayWorker();

        // We will assume that if it is not null, it is already initialized.
        penWorker->registerListener([](pen_event_t *event) {
            displayWorker->onPenEvent(event);
        });
    }
}

void disablePen() {
    if (displayWorker != nullptr) {
        auto *event = new pen_event_t();
        event->action = EXIT_WORKER;
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
    // TODO: implement nativeOnSizeChanged()
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