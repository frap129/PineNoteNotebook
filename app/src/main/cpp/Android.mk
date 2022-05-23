LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
uses-permission android:name="android.permission.ACCESS_SURFACE_FLINGER"
LOCAL_SRC_FILES:= \
	pinenotenotebook.cpp \
	pinenotelib.cpp \
	penworker.cpp \
	displayworker.cpp

#rgb888_to_gray_256_neon.s \

LOCAL_SHARED_LIBRARIES := \
	liblog

LOCAL_C_INCLUDES := \
	system/core/include/utils \
	frameworks/base/core/jni/include

LOCAL_CFLAGS := -Wall

LOCAL_MODULE:= libpinenote

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
