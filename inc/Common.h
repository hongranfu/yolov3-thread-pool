#ifndef __COMMON_H__
#define __COMMON_H__

#include <string>
#include <memory>
#include <vector>
#include <queue>
#include <functional>

#include <opencv2/opencv.hpp>
#include <gst/gst.h>
#include <json-glib/json-glib.h>

#include "jni_types.h"

//
// TS_ERR_MSG_V/TS_INFO_MSG_V/TS_WARN_MSG_V
//
#define TS_ERR_MSG_V(msg, ...)  \
    g_print("** ERROR: <%s:%s:%d>: " msg "\n", __FILE__, __func__, __LINE__, ##__VA_ARGS__)

#define TS_INFO_MSG_V(msg, ...) \
    g_print("** INFO:  <%s:%s:%d>: " msg "\n", __FILE__, __func__, __LINE__, ##__VA_ARGS__)

#define TS_WARN_MSG_V(msg, ...) \
    g_print("** WARN:  <%s:%s:%d>: " msg "\n", __FILE__, __func__, __LINE__, ##__VA_ARGS__)

typedef std::function<void(GstSample*, void*)> cbPutData;

typedef std::function<GstSample*(void*)> cbGetData;

typedef std::function<bool(const std::shared_ptr<std::vector<CLASSIFY_DATA> >&,
                void*)> cbPutResult;

typedef std::function<std::shared_ptr<std::vector<CLASSIFY_DATA> >
                (void*)> cbGetResult;

typedef std::function<void(GstBuffer*,
    const std::shared_ptr<std::vector<CLASSIFY_DATA> >&)> cbProcResult;

#endif // __COMMON_H__