package com.example.centos.tensorflowandroid;

import android.app.Activity;
import android.graphics.Bitmap;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;

public class MainActivity extends Activity implements CameraConnectionFragment.RegResultInterface, RegResultFragment.DataProvider{
    private String mIds;
    private Bitmap mImagebmp;

    @Override
    public void regSuc(String ids, Bitmap imageBitmap) {
        this.mIds = ids;
        this.mImagebmp = imageBitmap;
        getFragmentManager()
                .beginTransaction()
                .replace(R.id.container, new RegResultFragment())
                .addToBackStack(null)
                .commitAllowingStateLoss();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        getFragmentManager()
                .beginTransaction()
                .replace(R.id.container, CameraConnectionFragment.newInstance())
                .commit();
    }



    @Override
    public String getIds() {
        return mIds;
    }

    @Override
    public Bitmap getImageBmp() {
        return mImagebmp;
    }
}
