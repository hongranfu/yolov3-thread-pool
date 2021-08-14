/*
 * Copyright © 2021-2022 Thunder Software Technology Co., Ltd.
 * All rights reserved.
 * Author: qianyong
 * Date: 2021/06/25
 */
#ifndef __TS_DATA_DOUBLE_CACHE_H__
#define __TS_DATA_DOUBLE_CACHE_H__

//
// headers included
//
#include <chrono>
#include <functional>
#include <condition_variable>
#include <atomic>
#include <mutex>

#include <DataInterface.h>

#include <Common.h>

//
// DataDoubleCache<T>
//
template <typename T>
class DataDoubleCache : public DataInterface<T>
{
public:
    DataDoubleCache (
        std::function<bool(void)> notify) {
        notify_func_ = notify;
    }

   ~DataDoubleCache (void) {
    }

    virtual bool Post (const std::shared_ptr<T>& data) {
        if (!front_) {
            {
                std::lock_guard<std::mutex> lock (mutex_);
                front_ = data;
            }
            cond_.notify_one ();
        } else {
            mutex_.lock ();
            front_ = data;
            mutex_.unlock ();
        }

        if (DataInterface<T>::post_callback_) {
            DataInterface<T>::post_callback_ (data);
        }
        
        ready_ = TRUE;
        
        return TRUE;
    }
    
    virtual bool Pend (std::shared_ptr<T>& data, 
        int timeout = -1 /*unit:ms*/) {
        if (!front_) {
            std::unique_lock<std::mutex> lock (mutex_);
            
            if (timeout >= 0) {
                long int ns = static_cast<long int> (timeout*1e6);
                std::chrono::nanoseconds period = std::chrono::nanoseconds (ns);
                if (std::cv_status::timeout==cond_.wait_for (lock, period)) {
                    return FALSE;
                }
            } else {
                cond_.wait (lock);
            }
        }
        
        if (ready_) {
            mutex_.lock ();
            back_ = front_;
            mutex_.unlock ();
            ready_ = FALSE;
        }

        data = back_;

        if (DataInterface<T>::pend_callback_) {
            DataInterface<T>::pend_callback_ (data);
        }
        
        return TRUE;
    }

private:
    std::mutex                mutex_                ;
    std::condition_variable   cond_                 ;
    std::atomic<bool>         ready_       { FALSE };
    std::shared_ptr<T>        front_       {  NULL };
    std::shared_ptr<T>        back_        {  NULL };
    std::function<bool(void)> notify_func_ {  NULL };
};

#endif //__TS_DATA_DOUBLE_CACHE_H__

