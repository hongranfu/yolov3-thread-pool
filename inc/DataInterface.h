/*
 * Copyright © 2021-2022 Thunder Software Technology Co., Ltd.
 * All rights reserved.
 * Author: qianyong
 * Date: 2021/07/12
 */
#ifndef __TS_DATA_INTERFACE_H__
#define __TS_DATA_INTERFACE_H__

//
// headers included
//
#include <functional>
#include <memory>

#include <Common.h>

//
// DataInterface<T>
//
template <typename T>
class DataInterface
{
public:
    virtual bool Post (const std::shared_ptr<T>& data) {
        return FALSE;
    }
    
    virtual bool Pend (std::shared_ptr<T>& data, int timeout = -1) {
        return FALSE;
    }

    void SetPostCallback (
        const std::function<bool(const std::shared_ptr<T>&)>& postCallback) {
        post_callback_ = postCallback;
    }

    void SetPendCallback (
        const std::function<bool(const std::shared_ptr<T>&)>& pendCallback) {
        pend_callback_ = pendCallback;
    }

    DataInterface<T>* GetDataInterface (void) {
        return this;
    }

public:
    std::function<bool(const std::shared_ptr<T>&)> post_callback_ { NULL };
    std::function<bool(const std::shared_ptr<T>&)> pend_callback_ { NULL };
};

#endif //__TS_DATA_INTERFACE_H__

