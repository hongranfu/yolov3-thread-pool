/*
 * @Description: Implement of Data Mail box - a lock list template.
 * @version: 0.1
 * @Author: Ricardo Lu<shenglu1202@163.com>
 * @Date: 2021-08-14 19:12:19
 * @LastEditors: Ricardo Lu
 * @LastEditTime: 2021-08-15 13:56:15
 */


#ifndef __TS_DATA_MAILBOX_H__
#define __TS_DATA_MAILBOX_H__

#include <chrono>
#include <functional>
#include <condition_variable>
#include <mutex>
#include <list>

#include "DataInterface.h"

typedef enum {
  DISCARD_OLDEST,
  DISCARD_NEWEST
} DiscardStrategy;

template <typename T>
class DataMailbox : public DataInterface<T>
{
public:
    DataMailbox (
        DiscardStrategy strategy = DISCARD_OLDEST,
            int maxcnt = -1) :
        strategy_(strategy),
        data_maxcnt_(maxcnt) {
    }

    ~DataMailbox (void) {
    }

    virtual bool Post (
        const std::shared_ptr<T>& data) {
        {
            std::lock_guard<std::mutex> lock (mutex_);

            if (data_maxcnt_>0 && (int)list_.size() >= data_maxcnt_) {
                if (DISCARD_OLDEST == strategy_) {
                    list_.pop_front ();
                } else {
                    return false;
                }
            }

            list_.push_back (data);
        }

        condition_.notify_one ();

        if (DataInterface<T>::post_callback_) {
            DataInterface<T>::post_callback_ (data);
        }

        return true;
    }

    virtual bool Pend (
        std::shared_ptr<T>& data,
        int timeout = -1 /*unit:ms*/) {
        std::unique_lock<std::mutex> lock (mutex_);

        while (list_.size() == 0u) {
            if (timeout>=0) {
                long int ns = static_cast<long int>(timeout*1e6);
                std::chrono::nanoseconds period = std::chrono::nanoseconds (ns);
                if (std::cv_status::timeout==condition_.wait_for (lock, period)) {
                    return false;
                }
            } else {
                condition_.wait (lock);
            }
        }

        data = list_.front ();
        list_.pop_front ();

        if (DataInterface<T>::pend_callback_) {
            DataInterface<T>::pend_callback_ (data);
        }

        return true;
    }

    int Size(void) {
        std::lock_guard<std::mutex> lock (mutex_);

        return (int)list_.size();
    }

private:
    std::mutex                     mutex_     ;
    std::condition_variable        condition_ ;
    std::list<std::shared_ptr<T> > list_   { };
    bool strategy_          { DISCARD_OLDEST };
    int  data_maxcnt_       { -1             };

public:
    std::mutex& GetMutex (void) {
        return mutex_;
    }

    std::list<std::shared_ptr<T> >&
        GetList (void) {
        return list_;
    }
};

#endif //__TS_DATA_MAILBOX_H__

