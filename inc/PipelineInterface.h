#ifndef __TS_PIPELINE_INTERFACE_H__
#define __TS_PIPELINE_INTERFACE_H__

//
// headers included
//
#include "Common.h"

//
// SplCallbacks
//
typedef struct {
    cbPutData   putData    { NULL }; // args[0]
    cbGetResult  getResult  { NULL }; // args[1]
    cbProcResult procResult { NULL }; // args[2]
    void*        args[3]    { NULL, NULL, NULL };
} PipelineCallbacks;

//
// functions
//
extern "C"  void* splInit (const std::string& args);

extern "C"  bool splStart (void* spl);

extern "C"  bool splPause (void* spl);

extern "C"  bool splResume (void* spl);

extern "C"  void splStop (void* spl);

extern "C"  void splFina (void* spl);

extern "C"  bool splSetCb (void* spl, const PipelineCallbacks& cb);

#endif //__TS_PIPELINE_INTERFACE_H__
