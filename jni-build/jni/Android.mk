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
	./src/pcl/ocr/id/CardInfo.cpp \
	./src/pcl/ocr/id/classifier.cpp \
	./src/pcl/ocr/id/Detector.cpp \
	./src/pcl/ocr/id/ERState.cpp \
	./src/pcl/ocr/id/Divider.cpp \
	./src/pcl/ocr/id/Engine.cpp \
	./src/pcl/ocr/id/IDDecoder.cpp \
	./src/pcl/ocr/id/ImageUtil.cpp \
	./src/pcl/ocr/id/Rectifier.cpp \
	./src/pcl/ocr/id/Param.cpp \
	./jni_utils.cc \
	./imageutils_jni.cc \
	./yuv2rgb.cc


LOCAL_C_INCLUDES += $(LOCAL_PATH)/include

LOCAL_MODULE := tensorflow_ocr
LOCAL_ARM_MODE  := arm
NDK_MODULE_PATH := $(call my-dir)

include $(BUILD_SHARED_LIBRARY)
