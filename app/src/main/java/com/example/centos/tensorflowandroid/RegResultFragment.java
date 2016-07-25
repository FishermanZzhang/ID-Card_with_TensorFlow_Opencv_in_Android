package com.example.centos.tensorflowandroid;


import android.app.Activity;
import android.app.Fragment;
import android.content.Context;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

/**
 * 识别结果页面
 * <p>
 * Created by zczhang on 16/7/24.
 */
public class RegResultFragment extends Fragment {
    private TextView tvIds;
    private ImageView ivImage;
    private Button btnContinue;
    private DataProvider mDataProvider;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        return inflater.inflate(R.layout.fragment_reg_result, container, false);
    }

    @Override
    public void onViewCreated(View view, Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
        tvIds = (TextView) view.findViewById(R.id.tv_ids);
        ivImage = (ImageView) view.findViewById(R.id.iv_image);
        btnContinue = (Button) view.findViewById(R.id.btn_reg_continue);
        btnContinue.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                getFragmentManager()
                        .popBackStack();
            }
        });
    }

    @Override
    public void onAttach(Context context) {
        super.onAttach(context);
        if (context instanceof DataProvider) {
            this.mDataProvider = (DataProvider) context;
        }
    }

    @Override
    public void onAttach(Activity activity) {
        super.onAttach(activity);
        if (activity instanceof DataProvider) {
            this.mDataProvider = (DataProvider) activity;
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        if (mDataProvider != null) {
            tvIds.setText(mDataProvider.getIds());
            ivImage.setImageBitmap(mDataProvider.getImageBmp());
        }
    }

    public interface DataProvider {
        String getIds();

        Bitmap getImageBmp();
    }
}
