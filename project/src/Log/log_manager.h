//
// Created by lei on 2019/12/30.
//

#ifndef LEISQL_LOG_MANAGER_H
#define LEISQL_LOG_MANAGER_H

#include "../DiskManger/Disk_Manager.h"

namespace leisql{
    /**
     * 日志管理维持一个独立的线程
     * 如果日志缓冲区已满或者超时，这个线程就会唤醒
     * 将缓冲区的内容写到磁盘中
     */
    class LogManager {
    public:
        explicit LogManager(DiskManager *diskManager)
        :next_lsn_(0),persistent_lsn_(INVALID_LSN),diskManager(diskManager){
            log_buffer_=new char[LOG_BUFFER_SIZE];
            flush_buffer_=new char[LOG_BUFFER_SIZE];
        }

        ~LogManager(){
            delete [] log_buffer_;
            delete [] flush_buffer_;
            log_buffer_= nullptr;
            flush_buffer_= nullptr;
        }
    private:
        /**
         * The atomic counter which records the next log sequence number
         */
        std::atomic<lsn_t> next_lsn_;
        /**
         * The log records before and including the
         * persistent lsn have been written to disk
         */
        std::atomic<lsn_t> persistent_lsn_;



        char *log_buffer_;
        char *flush_buffer_;
        //__attribute__((__unused__))
        // 表示该函数或变量可能不使用，这个属性可以避免编译器产生警告信息
        DiskManager *diskManager  __attribute__((__unused__));


    };
}



#endif //LEISQL_LOG_MANAGER_H
