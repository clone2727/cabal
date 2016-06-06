LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

APP_ABI := $(ABI)
LOCAL_MODULE := cabalexec
LOCAL_SRC_FILES := ../libcabalexec.so

include $(PREBUILT_SHARED_LIBRARY)
