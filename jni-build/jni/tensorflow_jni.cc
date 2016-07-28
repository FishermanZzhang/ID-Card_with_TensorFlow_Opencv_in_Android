//
// Created by centos on 7/19/16.
//
// author zhangzhen

#include "tensorflow_jni.h"
#include "jni_utils.h"
#include "LSDDetectEdge.h"

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

using namespace tensorflow;
using namespace cv;
using namespace std;

static int g_tensorflow_input_height_size;
static int g_tensorflow_input_width_size;
static std::unique_ptr<tensorflow::Session> session;

static bool g_compute_graph_initialized = false;
static std::unique_ptr<LSDDetectEdge> detector;
static std::unique_ptr<std::string> g_input_name;
static std::unique_ptr<std::string> g_output_name;
//static std::unique_ptr<StatSummarizer> g_stats;
static const int kMaxChannelValue = 262143;

static const int NUMID = 18;
#ifndef MAX
#define MAX(a, b) ({__typeof__(a) _a = (a); __typeof__(b) _b = (b); _a > _b ? _a : _b; })
#define MIN(a, b) ({__typeof__(a) _a = (a); __typeof__(b) _b = (b); _a < _b ? _a : _b; })
#endif

// For basic benchmarking.
static int g_num_runs = 0;
static int64 g_timing_total_us = 0;
static Stat<int64> g_frequency_start;
static Stat<int64> g_frequency_end;

#ifdef LOG_DETAILED_STATS
static const bool kLogDetailedStats = true;
#else
static const bool kLogDetailedStats = false;
#endif

// Improve benchmarking by limiting runs to predefined amount.
// 0 (default) denotes infinite runs.
#ifndef MAX_NUM_RUNS
#define MAX_NUM_RUNS 0
#endif

#ifdef SAVE_STEP_STATS
static const bool kSaveStepStats = true;
#else
static const bool kSaveStepStats = false;
#endif

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

	//to do
	g_num_runs = 0;
	g_timing_total_us = 0;
	g_frequency_start.Reset();
	g_frequency_end.Reset();
	if (g_compute_graph_initialized) {
		LOG(INFO) << "Compute graph already loaded. skipping.";
		return 0;
	}
	const int64 start_time = CurrentThreadTimeUs();

	const char* const model_cstr = env->GetStringUTFChars(model, NULL);
	g_tensorflow_input_height_size = model_input_height_size;
	g_tensorflow_input_width_size = model_input_width_size;

	g_input_name.reset(
			new std::string(env->GetStringUTFChars(input_name, NULL)));
	g_output_name.reset(
			new std::string(env->GetStringUTFChars(output_name, NULL)));

	LOG(INFO) << "Loading TensorFlow.";
	LOG(INFO) << "Making new SessionOptions.";
	tensorflow::SessionOptions options;
	tensorflow::ConfigProto& config = options.config;
	LOG(INFO) << "Got config, " << config.device_count_size() << " devices";

	session.reset(tensorflow::NewSession(options));
	LOG(INFO) << "Session created.";
	tensorflow::GraphDef tensorflow_graph;
	LOG(INFO) << "Graph created.";

	AAssetManager* const asset_manager = AAssetManager_fromJava(env,
			java_asset_manager);
	LOG(INFO) << "Acquired AssetManager.";

	LOG(INFO) << "Reading file to proto: " << model_cstr;
	ReadFileToProto(asset_manager, model_cstr, &tensorflow_graph);

	//g_stats.reset(new StatSummarizer(tensorflow_graph));

	LOG(INFO) << "Creating session.";
	tensorflow::Status s = session->Create(tensorflow_graph);
	if (!s.ok()) {
		LOG(FATAL) << "Could not create TensorFlow Graph: " << s;
		return -1;
	}
	// Clear the proto to save memory space.
	tensorflow_graph.Clear();
	LOG(INFO) << "TensorFlow graph loaded from: " << model_cstr;

	LOG(INFO) << "init detector";
	detector.reset(new LSDDetectEdge);

	g_compute_graph_initialized = true;
	return 0;
}

static void line2blocks(cv::Mat& line, std::vector<cv::Rect>& blocks) {
    cv::Mat bin;
    if(line.channels() == 3){
      cv::cvtColor(line, line,CV_BGR2GRAY);
    }
    cv::threshold(line, bin, 0, 255, CV_THRESH_BINARY_INV | CV_THRESH_OTSU);
    vector<vector<cv::Point>> pointss;
    cv::findContours(bin, pointss, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
    //vector<cv::Rect> regions;
    for(auto& points : pointss){
        cv::Rect region;
        int minx = INT_MAX;
        int maxx = 0;
        int miny = INT_MAX;
        int maxy = 0;
        for(auto& point : points){
            minx = std::min(minx, point.x);
            maxx = std::max(maxx, point.x);
            miny = std::min(miny, point.y);
            maxy = std::max(maxy, point.y);
        }
        region.x = minx;
        region.y = miny;
        region.width = maxx - minx;
        region.height = maxy - miny;
        if(region.size().area() <= 10){
            continue;
        }
        blocks.emplace_back(region);
    }
    sort(blocks.begin(), blocks.end(), [](cv::Rect r1, cv::Rect r2){
        return r1.x < r2.x; });
}
//static void putCVMat2Tensor(tensor_map& input_tensor_mapped, int index, const cv::Mat& block){
//
//}

static std::string ClassifyImage(const cv::Mat& src) {
	//to do
	// Force the app to quit if we've reached our run quota, to make
	// benchmarks more reproducible.
	if (MAX_NUM_RUNS > 0 && g_num_runs >= MAX_NUM_RUNS) {
		LOG(INFO) << "Benchmark complete. "
				<< (g_timing_total_us / g_num_runs / 1000) << "ms/run avg over "
				<< g_num_runs << " runs.";
		LOG(INFO) << "more runing";
		//exit(0);
		return "!";
	}

	++g_num_runs;

	cv::Mat image;
	if (src.channels() == 4) {
		cv::cvtColor(src, image, CV_RGBA2GRAY);
	} else if (src.channels() == 3) {
		//cv::cvtColor(src, gray_image, CV_RGB2GRAY);
		image = src.clone();
	} else {
		image = src.clone();
	}
	vector<cv::Mat> items;
	LOG(INFO) << "detecting";
	detector->detect(image, items);
	LOG(INFO) << "detect over";
	if(items.size() <= 0){
	    return std::string("@");
	}
	vector<cv::Rect> blocks;
	//cv::Mat& line = items[0];
	line2blocks(items[0], blocks);

	if (blocks.size() < NUMID) {
		return std::string("#");
	}
	LOG(INFO) << "block nums" << blocks.size();
	// Create input tensor
	tensorflow::Tensor input_tensor(tensorflow::DT_FLOAT,
			tensorflow::TensorShape( { NUMID,
					g_tensorflow_input_width_size*g_tensorflow_input_height_size}));
	auto input_tensor_mapped = input_tensor.tensor<float, 2>();
	LOG(INFO) << "TensorFlow: Copying Data.";
    int start = std::max(0, (int) blocks.size() - NUMID);
	for (size_t index = start;
			index < blocks.size(); ++index) {
		cv::Mat stdBlock;
		cv::resize(items[0](blocks[index]), stdBlock,
				cv::Size(g_tensorflow_input_width_size,
						g_tensorflow_input_height_size), 0, 0, INTER_CUBIC);
		//LOG(INFO) << stdBlock.cols << "*" << stdBlock.rows;
		for (int i = 0; i < g_tensorflow_input_height_size; ++i) {

			// row ptr
			const uchar* imgptr = stdBlock.ptr<uchar>(i);
			for (int j = 0; j < g_tensorflow_input_width_size; ++j) {
				// Copy 1 values
				input_tensor_mapped(index - start, i *g_tensorflow_input_width_size +  j) = imgptr[j] / 255.0;
			}
		}
	}

	std::vector<std::pair<std::string, tensorflow::Tensor> > input_tensors( { {
			*g_input_name, input_tensor } });
	VLOG(0) << "Start computing.";
	std::vector<tensorflow::Tensor> output_tensors;
	std::vector<std::string> output_names( { *g_output_name });

	tensorflow::Status run_status;

	run_status = session->Run(input_tensors, output_names, { },
			&output_tensors);
	LOG(INFO) << "End computing.";
	if (!run_status.ok()) {
		LOG(ERROR) << "Error during inference: " << run_status;
		return std::string("$");
	}

	// Find best score digit
	std::string result;
	char label2char[] = "0123456789X";
	Tensor& output_tensor = output_tensors[0];
	auto  matrix = output_tensor.matrix<float>();
	LOG(INFO) << output_tensor.dim_size(0) << " " << output_tensor.dim_size(1);
	for (int index = 0; index < output_tensor.dim_size(0); ++index) {
		float max_score = std::numeric_limits<float>::min();
		int maxIndex = 0;
		for (int i = 0; i < output_tensor.dim_size(1); ++i) {
			const float score = matrix(index, i);
			if (score > max_score) {
				maxIndex = i;
				max_score = score;
			}
		}
		if(maxIndex < 0 || maxIndex >12){
		    maxIndex = 0;
		}
		//LOG(INFO) << "label " << maxIndex;
		result += label2char[maxIndex];
	}
	LOG(INFO) << result;
	return result;

}

/*
 * Class:     com_example_centos_tensorflowandroid_TensorFlowClassifier
 * Method:    classifyImageBmp
 * Signature: (Landroid/graphics/Bitmap;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_example_centos_tensorflowandroid_TensorFlowClassifier_classifyImageBmp(
		JNIEnv * env, jobject thiz, jobject bitmap) {
	//to do
	// Obtains the bitmap information.
	AndroidBitmapInfo info;
	CHECK_EQ(AndroidBitmap_getInfo(env, bitmap, &info),
			ANDROID_BITMAP_RESULT_SUCCESS);
	void* pixels;
	CHECK_EQ(AndroidBitmap_lockPixels(env, bitmap, &pixels),
			ANDROID_BITMAP_RESULT_SUCCESS);
	LOG(INFO) << "Image dimensions: " << info.width << "x" << info.height
			<< " stride: " << info.stride;
	// TODO(andrewharp): deal with other formats if necessary.
	if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
		LOG(FATAL) << "Only RGBA_8888 Bitmaps are supported.";
	}

	int size = info.width * info.height;
	Mat srcImage(info.height, info.width, CV_8UC4, (unsigned char*) pixels);
	LOG(INFO) << "srcimage" << srcImage.cols << " , " << srcImage.rows  << " " << srcImage.channels();
	imwrite("./1.jpg", srcImage);

	std::string result = ClassifyImage(srcImage);
	CHECK_EQ(AndroidBitmap_unlockPixels(env, bitmap),
			ANDROID_BITMAP_RESULT_SUCCESS);
	return env->NewStringUTF(result.c_str());

}

/*
 * Class:     com_example_centos_tensorflowandroid_TensorFlowClassifier
 * Method:    classifyImageRgb
 * Signature: ([III)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_example_centos_tensorflowandroid_TensorFlowClassifier_classifyImageRgb(
		JNIEnv * env, jobject thiz, jintArray image, jint width, jint height) {

	jboolean ptfalse = false;
	jint* srcBuf = env->GetIntArrayElements(image, &ptfalse);
	int size = width * height;
	Mat srcImage(height, width, CV_8UC4, (unsigned char*) srcBuf);

	std::string result = ClassifyImage(srcImage);
	env->ReleaseIntArrayElements(image, srcBuf, 0);
	return env->NewStringUTF(result.c_str());

}


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

	nR = MIN(kMaxChannelValue, MAX(0, nR));
	nG = MIN(kMaxChannelValue, MAX(0, nG));
	nB = MIN(kMaxChannelValue, MAX(0, nB));

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

