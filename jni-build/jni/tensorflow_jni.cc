//
// Created by centos on 7/19/16.
//
// author zhangzhen

#include "tensorflow_jni.h"
#include "pcl/ocr/id/card.h"

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/bitmap.h>

#include <jni.h>
#include <pthread.h>
#include <sys/stat.h>
#include <unistd.h>
#include <queue>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>


#include <opencv2/opencv.hpp>

#include "tensorflow/core/framework/step_stats.pb.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/framework/types.pb.h"
#include "tensorflow/core/lib/strings/stringprintf.h"
#include "tensorflow/core/platform/env.h"
#include "tensorflow/core/platform/logging.h"
#include "tensorflow/core/platform/mutex.h"
#include "tensorflow/core/platform/types.h"
#include "tensorflow/core/public/session.h"
#include "tensorflow/core/util/stat_summarizer.h"
#include "tensorflow/examples/android/jni/jni_utils.h"

using namespace tensorflow;

//using namespace tensorflow;
using namespace cv;
using namespace std;

std::unique_ptr<::pcl::ocr::id::Engine> engine;


// get the current thread time us
inline static int64 CurrentThreadTimeUs() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000000 + tv.tv_usec;
}

/*
 * Class:     com_example_centos_tensorflowandroid_TensorFlowClassifier
 * Method:    initializeTensorFlow
 * Signature: (Landroid/content/res/AssetManager;Ljava/lang/String;IIILjava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_example_centos_tensorflowandroid_TensorFlowClassifier_initializeTensorFlow(
		JNIEnv * env, jobject thiz, jobject java_asset_manager, jstring model,
		jint num_classes, jint model_input_height_size,
		jint model_input_width_size, jstring input_name, jstring output_name) {
	AAssetManager* const asset_manager = AAssetManager_fromJava(env,java_asset_manager);
	engine.reset(::pcl::ocr::id::Engine::Create(asset_manager));


	//int mkcode = mkdir("/data/data/com.example.centos.tensorflowandroid/files/imgs", 0777);
	//LOG(WARNING) << "mkdir error code" <<mkcode;
	return 0;
}


static std::string ClassifyImage(const cv::Mat& src) {
    LOG(INFO) << src.rows << "*"<< src.cols;
    float widthHeightRatio = 0.625f;//框框的宽高比
    int rectHeight =  src.rows * 3 / 5;//框框的高度
    int rectWidth = (int) (rectHeight / widthHeightRatio);//框框的宽度
    int leftTopPointY = (src.rows - rectHeight) / 2;//左上角Y坐标
    int leftTopPointX = (src.cols - rectWidth) / 2;//左上角X坐标

    cv::Rect roiId;
    int extendlen= 0;
    roiId.x = std::max(0, leftTopPointX - extendlen);
    roiId.y = std::max(0, leftTopPointY -extendlen);
    roiId.width = std::min(src.cols, rectWidth + 2 * extendlen);
    roiId.height = std::min(src.rows, rectHeight + 2 *extendlen);
    LOG(INFO) << roiId;
    //LOG(INFO) << engine;
    cv::Mat img;
    cv::resize(src(roiId).clone(),img,cv::Size(731, 472),0,0,CV_INTER_CUBIC);
    ::pcl::ocr::id::CardInfo info = engine->Recongnize(img);
    //LOG(INFO) <<info;
    std::string idname = info.id + " " + info.name;
    return idname;
    //return " ";
}

/*
 * Class:     com_example_centos_tensorflowandroid_TensorFlowClassifier
 * Method:    classifyImageBmp
 * Signature: (Landroid/graphics/Bitmap;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_example_centos_tensorflowandroid_TensorFlowClassifier_classifyImageBmp(
		JNIEnv * env, jobject thiz, jobject bitmap) {

	LOG(INFO) << "not implement";
	return env->NewStringUTF("");

}

/*
 * Class:     com_example_centos_tensorflowandroid_TensorFlowClassifier
 * Method:    classifyImageRgb
 * Signature: ([III)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_example_centos_tensorflowandroid_TensorFlowClassifier_classifyImageRgb(
		JNIEnv * env, jobject thiz, jintArray image, jint width, jint height) {
    LOG(INFO) << "not implement";
	return env->NewStringUTF("");

}


static const int kMaxChannelValue = 262143;
static inline uint32 YUV2RGB(int nY, int nU, int nV) {
	nY -= 16;
	nU -= 128;
	nV -= 128;
	if (nY < 0) nY = 0;

	// This is the floating point equivalent. We do the conversion in integer
	// because some Android devices do not have floating point in hardware.
	// nR = (int)(1.164 * nY + 2.018 * nU);
	// nG = (int)(1.164 * nY - 0.813 * nV - 0.391 * nU);
	// nB = (int)(1.164 * nY + 1.596 * nV);

	int nR = (int)(1192 * nY + 1634 * nV);
	int nG = (int)(1192 * nY - 833 * nV - 400 * nU);
	int nB = (int)(1192 * nY + 2066 * nU);

	nR = std::min(kMaxChannelValue, std::max(0, nR));
	nG = std::min(kMaxChannelValue, std::max(0, nG));
	nB = std::min(kMaxChannelValue, std::max(0, nB));

    /*
    nR = MIN(kMaxChannelValue, MAX(0, nR));
	nG = MIN(kMaxChannelValue, MAX(0, nG));
	nB = MIN(kMaxChannelValue, MAX(0, nB));
	*/

	nR = (nR >> 10) & 0xff;
	nG = (nG >> 10) & 0xff;
	nB = (nB >> 10) & 0xff;

	return 0xff000000 | (nR << 16) | (nG << 8) | nB;
}

/*
 * Class:     com_example_centos_tensorflowandroid_TensorFlowClassifier
 * Method:    classifyImageYUV
 * Signature: ([B[B[BIIIII)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_example_centos_tensorflowandroid_TensorFlowClassifier_classifyImageYUV
  (JNIEnv * env, jobject thiz,
  jbyteArray Y, jbyteArray U, jbyteArray V,
  jint width, jint height,
  jint y_row_stride, jint uv_row_stride, jint uv_pixel_stride){

    const uint8* yData = (uint8*)env->GetByteArrayElements(Y, NULL);
    const uint8* uData = (uint8*)env->GetByteArrayElements(U, NULL);
    const uint8* vData = (uint8*)env->GetByteArrayElements(V, NULL);
    // YUV420 to BGR
    Mat srcImage(height, width, CV_8UC3);
  	for (int y = 0; y < height; y++) {
  		const uint8* pY = yData + y_row_stride * y;
  		const int uv_row_start = uv_row_stride * (y >> 1);
  		const uint8* pU = uData + uv_row_start;
  		const uint8* pV = vData + uv_row_start;
  		cv::Vec3b* data = srcImage.ptr<cv::Vec3b>(y);
  		for (int x = 0; x < width; x++) {
  			const int uv_offset = (x >> 1) * uv_pixel_stride;
  			int ABGR= YUV2RGB(pY[x], pU[uv_offset], pV[uv_offset]);
  			data[x][0] = ABGR & 0xff;
  			data[x][1] = (ABGR & 0xff00) >> 8;
  			data[x][2] = (ABGR & 0xff0000) >> 16;
  		}
  	}

    std::string result = ClassifyImage(srcImage);
    env->ReleaseByteArrayElements(Y, (jbyte*)yData, 0);
    env->ReleaseByteArrayElements(U, (jbyte*)uData, 0);
    env->ReleaseByteArrayElements(V, (jbyte*)vData, 0);
    return env->NewStringUTF(result.c_str());
  }

