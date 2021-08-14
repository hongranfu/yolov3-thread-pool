//ThreadPool.h

#pragma once 
#include <functional> 
#include <future> 
#include <mutex> 
#include <queue> 
#include <thread> 
#include <utility> 
#include <vector>

#include "SafeQueue.h"
#include "YoloClassification.h"
#include "Common.h"

class ThreadPool {
private:
    class ThreadWorker {//内置线程工作类

    private:
        int m_id; //工作id

        ThreadPool* m_pool;//所属线程池

        std::shared_ptr<YoloClassification> m_yolov3;
        cbPutResult m_PutResultCallback;
        void* m_PutResultArg;

    public:
        //构造函数
        ThreadWorker(ThreadPool* pool, const int id, const float conf)
            :m_pool(pool), m_id(id) {
            m_yolov3 = std::make_shared<YoloClassification> ();
            m_yolov3->init(AIP);
            m_yolov3->setConfidence(conf);
        }

        void SetCallback(cbPutResult func, void* arg) {
            m_PutResultCallback = func;
            m_PutResultArg = arg;
        }

        //重载`()`操作
        void operator()() {
            std::shared_ptr<cv::Mat> image;

            bool dequeued; //是否正在取出队列中元素

            //判断线程池是否关闭，没有关闭，循环提取
            while (!m_pool->m_shutdown) {
                {
                    //为线程环境锁加锁，互访问工作线程的休眠和唤醒
                    std::unique_lock<std::mutex> lock(m_pool->m_conditional_mutex);

                    //如果任务队列为空，阻塞当前线程
                    if (m_pool->m_queue.empty()) {
                        //等待条件变量通知，开启线程
                        m_pool->m_conditional_lock.wait(lock); 
                    }

                    //取出任务队列中的元素
                    dequeued = m_pool->m_queue.dequeue(image);
                }
 
                //如果成功取出，识别图片并将识别结果通过回调入队
                if (dequeued) {
                    std::vector<CLASSIFY_DATA> result =  m_yolov3->doDetect(image);
                    // TS_INFO_MSG_V ("thread is: %d, Detect %ld results, first label: %d.",
                    //     m_id, result.size(), result[0].label);

                    m_PutResultCallback(std::make_shared<
                        std::vector<CLASSIFY_DATA> >(result), m_PutResultArg);
                }
            }
        }
    };

    bool m_shutdown; //线程池是否关闭

    SafeQueue<cv::Mat> m_queue;//执行函数安全队列，即任务队列

    std::vector<std::thread> m_threads; //工作线程队列

    std::mutex m_conditional_mutex;//线程休眠锁互斥变量

    std::condition_variable m_conditional_lock; //线程环境锁，让线程可以处于休眠或者唤醒状态

public:
    //线程池构造函数
    ThreadPool(const int n_threads)
        : m_threads(std::vector<std::thread>(n_threads)), m_shutdown(false) {
    }

    ThreadPool(const ThreadPool &) = delete; //拷贝构造函数，并且取消默认父类构造函数

    ThreadPool(ThreadPool &&) = delete; // 拷贝构造函数，允许右值引用

    ThreadPool & operator=(const ThreadPool &) = delete; // 赋值操作

    ThreadPool & operator=(ThreadPool &&) = delete; //赋值操作

    // Inits thread pool
    void init(const float conf, cbPutResult func, void* arg) {
        for (int i = 0; i < m_threads.size(); ++i) {
            ThreadWorker tw(this, i, conf);
            tw.SetCallback(func, arg);
            m_threads[i] = std::thread(tw);//分配工作线程
        }
    }

    // Waits until threads finish their current task and shutdowns the pool
    void shutdown() {
        m_shutdown = true;
        m_conditional_lock.notify_all(); //通知 唤醒所有工作线程

        for (int i = 0; i < m_threads.size(); ++i) {
            if(m_threads[i].joinable()) { //判断线程是否正在等待
                m_threads[i].join();  //将线程加入等待队列
            }
        }
    }

    template<typename T>
    void submit(const std::shared_ptr<T>& data) {
        m_queue.enqueue(data);
        TS_INFO_MSG_V ("thread work queue submit task, current tasks: %d",
            m_queue.size());
        m_conditional_lock.notify_one();
    }
};
