package com.example.centos.tensorflowandroid;

/**
 * Created by centos on 7/12/16.
 */


import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.media.Image;
import android.media.ImageReader;
import android.media.ImageReader.OnImageAvailableListener;
import android.os.Handler;
import android.os.Trace;
import android.util.Log;

import java.nio.ByteBuffer;

public class ImageListener implements OnImageAvailableListener {
    private static final String TAG = "ImageListener";
    private Handler handler;
    private CameraConnectionFragment fragment;
    private Integer sensorOrientation;
    Image image;
    Bitmap bitmap;

    private static final int NUM_CLASSES = 11;
    private static final int INPUT_SIZE = 32;
    private static final String INPUT_NAME = "input";
    private static final String OUTPUT_NAME = "output";
    private static final String MODEL_FILE = "file:///android_asset/expert-graph.pb";

    private final TensorFlowClassifier tensorflow = new TensorFlowClassifier();
    private boolean computing = false;
    private boolean computeSuccess = false;
    private String imageFileName = "ids.png";
    private String beforeImageFileName = "ids_before.jpg";
    private String mFilePath = "sdcard/";
    private byte[] bytes;

    public void initialize(
            final AssetManager assetManager,
            final Handler handler,
            final Integer sensorOrientation, CameraConnectionFragment fragment) {
        //Assert.assertNotNull(sensorOrientation);
        tensorflow.initializeTensorFlow(assetManager, MODEL_FILE, NUM_CLASSES, INPUT_SIZE, INPUT_SIZE, INPUT_NAME, OUTPUT_NAME);
        this.handler = handler;
        this.sensorOrientation = sensorOrientation;
        this.fragment = fragment;
        computeSuccess = false;
    }
    /*
    public void initialize(
            final AssetManager assetManager,
            final Handler handler,
            final Integer sensorOrientation) {
        //Assert.assertNotNull(sensorOrientation);
        tensorflow.initializeTensorFlow(assetManager,MODEL_FILE,NUM_CLASSES,INPUT_SIZE, INPUT_SIZE,INPUT_NAME,OUTPUT_NAME);
        this.handler = handler;
        this.sensorOrientation = sensorOrientation;
    }*/

    @Override
    public void onImageAvailable(final ImageReader reader) {
        //backgroundHandler.post(new ImageSaver(reader.acquireNextImage(), mFile));
        Log.i(TAG, "onImageAvailable: 当前线程---->" + Thread.currentThread().getName());
        image = null;
        try {
            image = reader.acquireLatestImage();
            if (image == null) {
                return;
            }
            if (computing || computeSuccess) {
                image.close();
                return;
            }
            computing = true;
            Image.Plane[] planes = image.getPlanes();
            int hi = image.getHeight();
            int wi = image.getWidth();
            ByteBuffer buffer = planes[0].getBuffer();
            int pixelStride = planes[0].getPixelStride();
            int rowStride = planes[0].getRowStride();
            bitmap = Bitmap.createBitmap(
                    wi, hi, Config.ARGB_8888
            );


            int offset = 0;
            int rowPadding = rowStride - pixelStride * wi;
            //System.out.println("pixelStride, rowStride, rowPadding) : " + pixelStride + ", " + rowStride + " , " + rowPadding);
            for (int i = 0; i < hi; ++i) {
                for (int j = 0; j < wi; ++j) {
                    int pixel = 0;
                    pixel |= (buffer.get(offset) & 0xff) << 16;     // R
                    pixel |= (buffer.get(offset) & 0xff) << 8;  // G
                    pixel |= (buffer.get(offset) & 0xff);       // B
                    pixel |= (buffer.get(offset) & 0xff) << 24; // A
                    bitmap.setPixel(j, i, pixel);
                    offset += pixelStride;
                }
                offset += rowPadding;
            }
//            Trace.beginSection("imageAvailable");
            //int rowPadding = rowStride - pixelStride * mWidth;
            //bitmap = Bitmap.createBitmap()
            //System.out.println("bitmap height" + bitmap.getHeight());

        } catch (final Exception e) {
            if (image != null) {
                image.close();
            }
            //LOGGER.e(e, "Exception!");
            Trace.endSection();
            return;
        } finally {
            if (image != null) {
                image.close();
            }
        }
        handler.post(
                new Runnable() {
                    @Override
                    public void run() {
                        Log.i(TAG, "onImageAvailable: 当前线程2---->" + Thread.currentThread().getName());
                        //final List<Classifier.Recognition> results = tensorflow.recognizeImage(croppedBitmap);

                        //LOGGER.v("%d results", results.size());
                        //for (final Classifier.Recognition result : results) {
                        //LOGGER.v("Result: " + result.getTitle());
                        //}
                        //scoreView.setResults(results);
                        //computing = false;
                        //Log.d(TAG, "run: ");

                        System.out.println("bitmap height" + bitmap.getHeight());
                        String ids = tensorflow.classifyImageBmp(bitmap);
                        //String ids = tensorflow.classifyImageBmp(bytes, image.getWidth(), image.getHeight());
                        System.out.println(ids);
                        computing = false;
                        if (ids.length() != 18) {
                            return;
                        }
                        computeSuccess = true;
//                        Utils.saveBitmap2File(bitmap, mFilePath, imageFileName);

                        //final Activity activity = getActivity();
                        fragment.setResult(ids, bitmap);
                    }
                });
    }
}
