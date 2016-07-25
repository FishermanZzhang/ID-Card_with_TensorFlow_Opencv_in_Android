ID-Card_with_TensorFlow_Opencv_in_Android
==============================

# Environment
####1 centos 7
####2 android studio
####3 NDK 
####4 SDK
####5 JDK 1.8


# download tensorflow-sdk and opencv-sdk
####1 可以直接下载[tensorflow-sdk]() 
也可以自己编译tensorflow android "static lib". 步骤如下：

[tensorflow_android]()按照说明，并安装bazel等。

在[WORKSPACE](https://github.com/tensorflow/tensorflow/blob/master/WORKSPACE)中 配置SDK 和 NDK（NDK 最好选择 10e,NDK12 就不行)

在[BUILD](https://github.com/tensorflow/tensorflow/blob/master/tensorflow/BUILD)中配置cpu平台

```
config_setting(
  name = "android_arm",
  values = {
    "crosstool_top": "//external:android/crosstool",
    "android_cpu": "armeabi-v7a",
  },
  visibility = ["//visibility:public"],
)
```


####2 下载[opencv-sdk]()

####3 仔细阅读Opencv.mk 和 tensorflow_android.mk(这个是我写的，可能有bug,在arm平台是没有问题的)

####4 


# use it
