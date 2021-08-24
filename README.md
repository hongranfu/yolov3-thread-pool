# yolov3-thread-pool
[TOC]

## Overview

本仓库是[Ericsson-Yolov3-SNPE](https://github.com/gesanqiu/Ericsson-Yolov3-SNPE)的优化版本，在使用新框架的基础上新增了一些特性：

- 使用GFlags完成命令行参数的设置；
- 通过读取`json`文件完成程序`Gstreamer pipeline`的配置；
- 抽象标准接口用于控制`Gstreamer pipeline`；
- 引入[draw-rectangle-on-YUV](https://github.com/gesanqiu/draw-rectangle-on-YUV)用于完成在`NV12`图像上画框；
- **使用线程池管理Yolov3算法处理线程。**

### 文件树

```shell
# All source code files have describe at the top of the file.
.
├── CMakeLists.txt
├── configs
│   └── yolov3.json						# pipeline config file
├── inc
│   ├── Common.h
│   ├── DataDoubleCache.h
│   ├── DataInterface.h
│   ├── DataMailbox.h
│   ├── jni_types.h
│   ├── MyUdlLayers.h
│   ├── PipelineInterface.h
│   ├── SafeQueue.h
│   ├── snpe_udl_layers.h
│   ├── ThreadPool.h
│   ├── TimeUtil.h
│   ├── VideoPipeline.h
│   └── YoloClassification.h
├── LICENSE
├── README.md
└── src
    ├── main.cpp
    ├── PipelineInterface.cpp
    ├── VideoPipeline.cpp
    └── YoloClassification.cpp
```

### 开发平台

- 开发平台：Qualcomm® QRB5165 (Linux-Ubuntu 18.04)
- 图形界面：Weston(Wayland)
- 开发框架：Gstreamer，OpenCV-4.2.0
- 算法框架：SNPE
- 算法示例模型：YoloV3
- 第三方库：gflags，json-glib-1.0，glib-2.0
- 构建工具：CMake

## 运行流程

![SampleFrame](images/SampleFrame.png)

- 解码通过`uridecodebin`完成；
- 使用`tee`插件解耦视频处理和算法处理，维护两个数据缓冲队列完成两者必须的数据交互；
- 给`queue0`插件的`src pad`增加一个probe回调，用于完成图像的画框。

## 线程池

由[Ericsson-Yolov3-SNPE](https://github.com/gesanqiu/Ericsson-Yolov3-SNPE)的测试数据可知，当前算法AIP Runtime的单帧(1920x1080 ppi)处理耗时大约在95ms，并且多实例会导致性能损耗，因此为了达到需求的25FPS实时性能我最少得开辟4个AIP算法实例，而保险起见我初始化了6个并且使用`switch-case`这种十分粗糙的手段完成了多线程的数据同步。

最近闲暇时间学习了线程池的实现，有关线程池的教程可以参考[Thread Pool in C++11](https://ricardolu.gitbook.io/trantor/c++-thread-pool)，在模板中我们从外部传递一个工作函数和相关参数给到线程池，随后唤起空闲线程去执行提交的工作函数。其核心就是一个循环论询的任务队列和多个用于完成任务的工作线程，那么借鉴任务队列的设计思想，**我将算法实例捆绑到工作线程中（固定任务类型，只提交任务所需的参数）**，将pipeline中`appsink`插件吐出来的图像作为任务submit到线程池以此唤醒唤醒空闲（当前未执行/已完成识别任务）的算法实例进行识别，让线程池自动按顺序去完成识别任务，对比旧版在一定程度上降低了空间开销。

- 使用线程池，相当于异步处理图像，由于每个线程中算法实例耗费的识别时间不同，因此存储识别结果的`DataMailBox`中存储的结果向量并不严格符合图像显示顺序，但是差距大致在20ms以内，用户感知并不明显，算是一种妥协。

## 算法处理帧率控制

```json
{
	"fps-n":25,
    "fps-d":5
}
```

Gstreamer Pipeline新增两个参数`output_fps_n_`和`output_fps_d_`，用于控制appsink回调调用`put_frame_func_()`送算法帧率（两个参数的商为算法处理的帧率）。

```c++
if (sample) {
    // control the frame rate of video submited to analyzer
    if (0 == vp->first_frame_timestamp_) {
        vp->first_frame_timestamp_ = g_get_real_time ();
    } else {
        gint64 period_us_count = g_get_real_time () - vp->first_frame_timestamp_;
        double cur = (double)(vp->appsinked_frame_count_*1000*1000) / (double)(period_us_count);
        double dst = (double)(vp->config_.output_fps_n_) / (double)(vp->config_.output_fps_d_);
        // TS_INFO_MSG_V ("%2.2f, %2.2f, %d/%d", cur, dst, vp->config_.output_fps_n_,
        //    vp->config_.output_fps_d_);

        if (cur > dst) {
            gst_sample_unref (sample);
            return GST_FLOW_OK;
        }
    }
    
    if (vp->put_frame_func_) {
        vp->put_frame_func_(sample, vp->put_frame_args_);
        vp->appsinked_frame_count_++;
    } else {
        gst_sample_unref (sample);
    }
}
```

维持一个算法已处理帧计数`appsinked_frame_count_`(s)和当前帧耗时`period_us_count`(us)。假设当前要控制算法处理帧率为5FPS，即`dst=5`，每sink一帧相当于增加1s，在5帧内，只要视频源的速度大于5FPS，`cur`都会大于5，这时直接释放`GstSample`并返回；直到第6帧，`cur`将小于5，送算法并且计数器加1，以此实现均匀跳帧。

## qtioverlay

`qtioverlay`是高通平台上用于给`NV12`格式图片帧绘画的一个Gstreamer插件，它读取GstBuffer中的metadata然后使用GPU完成Rectangle和name(string)的绘制。

```c++
#include <ml-meta/ml_meta.h> // -l libqtimlmeta.so

// in osdResult(): replace drawYUVRect() to followning line
{
    GstMLDetectionMeta *meta = gst_buffer_add_detection_meta(buffer);
    if (!meta) {
        TS_ERR_MSG_V ("Failed to create metadata");
        return ;
    }

    GstMLClassificationResult *box_info = (GstMLClassificationResult*)malloc(
        sizeof(GstMLClassificationResult));

    uint32_t label_size = g_labels[results->at(i).label].size() + 1;
    box_info->name = (char *)malloc(label_size);
    snprintf(box_info->name, label_size, "%s", g_labels[results->at(i).label].c_str());

    box_info->confidence = results->at(i).confidence;
    meta->box_info = g_slist_append (meta->box_info, box_info);

    meta->bounding_box.x = results->at(i).rect[0];
    meta->bounding_box.y = results->at(i).rect[1];
    meta->bounding_box.width = results->at(i).rect[2];
    meta->bounding_box.height = results->at(i).rect[3];
}

```

## 编译&运行

```shell
cmake -H. -Bbuild
cd build && make

# --cfg：pipeline 所用配置文件，用于完成VideoPipelineConfigure结构体的初始化
# --count:yolov3算法实例个数
./AiObject-Yolov3 --cfg ../configs/yolov3.json --count 4
```

## FAQ

- 由于使用新框架直接在NV12图像上画框，因此无法将识别出的物体类别绘制在图像上，有需求可以使用OpenGL绘制。