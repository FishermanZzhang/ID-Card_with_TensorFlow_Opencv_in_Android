LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#opencv
OPENCV_CAMERA_MODULES:=on
OPENCV_INSTALL_MODULES:=on
OPENCV_LIB_TYPE:=STATIC
include /home/centos/sdk/OpenCV-android-sdk/sdk/native/jni/OpenCV.mk

#tf-android
include /home/centos/sdk/tensorflow-sdk/sdk/native/jni/tensorflow_android.mk

LOCAL_SRC_FILES := ./tensorflow_jni.cc \
	./jni_utils.cc \
	./LSDDetectEdge.cc \
	./DetectEdge.cc

LOCAL_MODULE := tensorflow_ocr
LOCAL_ARM_MODE  := arm
NDK_MODULE_PATH := $(call my-dir)

include $(BUILD_SHARED_LIBRARY)
