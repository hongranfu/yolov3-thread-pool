/*
 * @Description: Standard pipeline library abstrct.
 * @version: 0.1
 * @Author: Ricardo Lu<shenglu1202@163.com>
 * @Date: 2021-08-14 19:12:19
 * @LastEditors: Ricardo Lu
 * @LastEditTime: 2021-08-15 13:56:04
 */


#ifndef __TS_PIPELINE_INTERFACE_H__
#define __TS_PIPELINE_INTERFACE_H__

#include "Common.h"

typedef struct {
    cbPutData   putData    { NULL }; // args[0]
    cbGetResult  getResult  { NULL }; // args[1]
    cbProcResult procResult { NULL }; // args[2]
    void*        args[3]    { NULL, NULL, NULL };
} PipelineCallbacks;

extern "C"  void* splInit (const std::string& args);

extern "C"  bool splStart (void* spl);

extern "C"  bool splPause (void* spl);

extern "C"  bool splResume (void* spl);

extern "C"  void splStop (void* spl);

extern "C"  void splFina (void* spl);

extern "C"  bool splSetCb (void* spl, const PipelineCallbacks& cb);

#endif //__TS_PIPELINE_INTERFACE_H__
