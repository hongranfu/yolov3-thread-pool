/*
 * @Description: Adavanced data buffer structure APIs.
 * @version: 0.1
 * @Author: Ricardo Lu<shenglu1202@163.com>
 * @Date: 2021-08-14 19:12:19
 * @LastEditors: Ricardo Lu
 * @LastEditTime: 2021-08-15 13:56:52
 */

#ifndef __TS_DATA_INTERFACE_H__
#define __TS_DATA_INTERFACE_H__

#include <functional>
#include <memory>

#include <Common.h>

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

