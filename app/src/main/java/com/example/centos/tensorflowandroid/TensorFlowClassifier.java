package com.example.centos.tensorflowandroid;

import android.content.res.AssetManager;
import android.graphics.Bitmap;

/**
 * Created by centos on 7/19/16.
 */
public class TensorFlowClassifier {
    private static final String TAG = "TensorflowClassifier";


    // jni native methods.
    public native int initializeTensorFlow(
            AssetManager assetManager,
            String model,
            int numClasses,
            int inputHeigth,
            int inputWidth,
            String inputName,
            String outputName);
    public native String classifyImageBmp(Bitmap bitmap);

    public native String classifyImageRgb(int[] output, int width, int height);
    public native String classifyImageYUV(byte[] y, byte[] u, byte[] v,
                                          int width, int hegiht ,
                                          int yRowStride, int uvRowStride, int uvPixelStride);


    static {
        System.loadLibrary("tensorflow_ocr");
    }

}
