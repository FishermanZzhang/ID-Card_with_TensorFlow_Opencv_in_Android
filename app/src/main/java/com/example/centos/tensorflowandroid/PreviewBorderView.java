package com.example.centos.tensorflowandroid;

import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PixelFormat;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffXfermode;
import android.graphics.RectF;
import android.util.AttributeSet;
import android.util.Log;
import android.util.TypedValue;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

/**
 * Created by zczhang on 16/8/9.
 */
public class PreviewBorderView extends SurfaceView implements SurfaceHolder.Callback, Runnable {
    private static final String TAG = "PreviewBorderView";
    private int mScreenH;
    private int mScreenW;
    private Canvas mCanvas;
    private Paint mPaint;
    private Paint mPaintLine;
    private SurfaceHolder mHolder;
    private Thread mThread;
    private static final String DEFAULT_TIPS_TEXT = "请将方框对准证件拍摄";
    private static final int DEFAULT_TIPS_TEXT_SIZE = 16;
    private static final int DEFAULT_TIPS_TEXT_COLOR = Color.GREEN;
    private static final int DEDAULT_LINE_LENGTH = 50;//框框四个角需要划得线的长度
    /**
     * 自定义属性
     */
    private float tipTextSize;
    private int tipTextColor;
    private String tipText;

    public PreviewBorderView(Context context) {
        this(context, null);
    }

    public PreviewBorderView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public PreviewBorderView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        initAttrs(context, attrs);
        init();
    }

    /**
     * 初始化自定义属性
     *
     * @param context Context
     * @param attrs   AttributeSet
     */
    private void initAttrs(Context context, AttributeSet attrs) {
        TypedArray a = context.obtainStyledAttributes(attrs, R.styleable.PreviewBorderView);
        try {
            tipTextSize = a.getDimension(R.styleable.PreviewBorderView_tipTextSize, TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, DEFAULT_TIPS_TEXT_SIZE, getResources().getDisplayMetrics()));
            tipTextColor = a.getColor(R.styleable.PreviewBorderView_tipTextColor, DEFAULT_TIPS_TEXT_COLOR);
            tipText = a.getString(R.styleable.PreviewBorderView_tipText);
            if (tipText == null) {
                tipText = DEFAULT_TIPS_TEXT;
            }
        } finally {
            a.recycle();
        }


    }

    /**
     * 初始化绘图变量
     */
    private void init() {
        this.mHolder = getHolder();
        this.mHolder.addCallback(this);
        this.mHolder.setFormat(PixelFormat.TRANSPARENT);
        setZOrderOnTop(true);
        this.mPaint = new Paint();
        this.mPaint.setAntiAlias(true);
        this.mPaint.setColor(Color.WHITE);
        this.mPaint.setStyle(Paint.Style.FILL_AND_STROKE);
        this.mPaint.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.CLEAR));
        this.mPaintLine = new Paint();
        this.mPaintLine.setColor(tipTextColor);
        this.mPaintLine.setStrokeWidth(3.0F);
        setKeepScreenOn(true);
    }

    /**
     * 绘制取景框
     */
    private void draw() {
        try {
            this.mCanvas = this.mHolder.lockCanvas();
            this.mCanvas.drawARGB(100, 0, 0, 0);
            float widthHeightRatio = 0.75f;//框框的宽高比
            int rectHeight = this.mScreenH * 3 / 4;//框框的高度
            int rectWidth = (int) (rectHeight / widthHeightRatio);//框框的宽度
            int leftTopPointY = this.mScreenH / 8;//左上角Y坐标
            int leftTopPointX = (this.mScreenW - rectWidth) / 2;//左上角X坐标
            int leftBottomPointY = this.mScreenH * 7 / 8;//左下角Y坐标
            int leftBottomPointX = leftTopPointX;//左下角X坐标
            int rightTopY = leftTopPointY;//右上角Y坐标
            int rightTopX = leftTopPointX + rectWidth;//右上角X坐标
            int rightBottomY = leftBottomPointY;//右下角Y坐标
            int rightBottomX = rightTopX;//右下角X坐标
            this.mCanvas.drawRect(new RectF(leftTopPointX, leftTopPointY, rightBottomX, rightBottomY), this.mPaint);

            this.mCanvas.drawLine(leftTopPointX, leftTopPointY, leftTopPointX + DEDAULT_LINE_LENGTH, leftTopPointY, this.mPaintLine);
            this.mCanvas.drawLine(leftTopPointX, leftTopPointY, leftTopPointX , leftTopPointY + DEDAULT_LINE_LENGTH, this.mPaintLine);

            this.mCanvas.drawLine(leftBottomPointX, leftBottomPointY, leftBottomPointX + DEDAULT_LINE_LENGTH, leftBottomPointY, this.mPaintLine);
            this.mCanvas.drawLine(leftBottomPointX, leftBottomPointY, leftBottomPointX , leftBottomPointY - DEDAULT_LINE_LENGTH, this.mPaintLine);

            this.mCanvas.drawLine(rightTopX, rightTopY, rightTopX - DEDAULT_LINE_LENGTH, rightTopY, this.mPaintLine);
            this.mCanvas.drawLine(rightTopX, rightTopY, rightTopX , rightTopY + DEDAULT_LINE_LENGTH, this.mPaintLine);

            this.mCanvas.drawLine(rightBottomX, rightBottomY, rightBottomX - DEDAULT_LINE_LENGTH, rightBottomY, this.mPaintLine);
            this.mCanvas.drawLine(rightBottomX, rightBottomY, rightBottomX , rightBottomY - DEDAULT_LINE_LENGTH, this.mPaintLine);

            mPaintLine.setTextSize(tipTextSize);
            mPaintLine.setAntiAlias(true);
            mPaintLine.setDither(true);
            float length = mPaintLine.measureText(tipText);
            this.mCanvas.drawText(tipText, leftTopPointX + rectWidth / 2 - length / 2, leftTopPointY - tipTextSize, mPaintLine);
            Log.e(TAG, "draw: left = " + leftTopPointX );
            Log.e(TAG, "draw: top = " + leftTopPointY );
            Log.e(TAG, "draw: right = " + rightBottomX );
            Log.e(TAG, "draw: bottom = " + rightBottomY );
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            if (this.mCanvas != null) {
                this.mHolder.unlockCanvasAndPost(this.mCanvas);
            }
        }
    }


    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        Log.i(TAG, "surfaceCreated: width = " + getWidth());
        //获得宽高，开启子线程绘图
//        this.mScreenW = getWidth();
//        this.mScreenH = getHeight();
//        this.mThread = new Thread(this);
//        this.mThread.start();
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        //停止线程
        try {
            mThread.interrupt();
            mThread = null;
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    @Override
    public void run() {
        //子线程绘图
        draw();
    }

    public void setSize(int width, int height) {
        Log.i(TAG, "setSize: width = " + width + ";height = " + height);
        this.mScreenW = width;
        this.mScreenH = height;
        this.mThread = new Thread(this);
        this.mThread.start();
    }
}

