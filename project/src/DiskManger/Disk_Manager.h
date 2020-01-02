//
// Created by lei on 2019/12/29.
//

#ifndef LEISQL_DISK_MANAGER_H
#define LEISQL_DISK_MANAGER_H
/**
 * 对磁盘文件进行操作
 */
#include "../common/config.h"
#include <iostream>
#include <future>
#include <fstream>

namespace leisql{
    class DiskManager {
    public:
        DiskManager(const std::string &db_file);
        ~DiskManager();

        /**
            对磁盘上的page 写入操作
         * @param pageId
         * @param page_data
         */
        void WritePage(page_id_t pageId,const char *page_data);

        /**
         * 根据page_id 读出磁盘中page 内容到内存，将内存地址赋值给 *page_data
         * @param pageId
         * @param page_data
         */
        void ReadPage(page_id_t pageId,char *page_data);

        /**
         * 追加日志到日志文件末尾
         * @param log_data
         * @param size
         */
        void WriteLog(char *log_data,int size);

        /**
         * 从日志文件中读取一部分
         * @param log_data  output buffer
         * @param size size of the log entry
         * @param offset offset of the log entry in the file
         * @return true if the read was successful, false otherwise
         */
        bool ReadLog(char *log_data,int size,int offset);

        /**
         *  将页面在磁盘上抹掉
         * @param pageId
         */
        void DeallocatePage(page_id_t pageId);

        /**
         * disk flushes 的次数
         * @return
         */
        int GetNumFlushes() const;

        /**
         * true if the in-memory content has not been flushed yet
         * @return
         */
        bool GetFlushesState()const;

        /**
         * 磁盘写的次数
         * @return
         */
        int GetNumWrites()const;

        /**
         * 多线程输出没办法控制输出的顺序，
         * 那么future就为所有会有输出的位置放一个占位符，
         * 认定这个位置会有一个输出，但是具体什么时间会有不确定。
         *
         * 将f中的内容 写到数据块flush_log_f_
         *
         */
        inline void SetFlushLogFuture(std::future<void> *f){
            flush_log_f_ = f;
        }

        /**
         * 判断是否有block_flush发生
         */
        inline bool HashFlushLogFuture(){
            return flush_log_f_!= nullptr;
        }

        /**
         * 关闭所有文件流
         */
        void Shutdown();


        /**
         * Allocate a page on disk
         * @return the id of the allocated page
         */
         page_id_t AllocatePages();





    private:
        int GetFileSize(const std::string &file_name);

        //stream to write log file
        std::fstream log_io_;
        std::string log_name_;

        //stream to write db file
        std::fstream  db_io_;
        std::string file_name_;

        //原子数据类型，避免多线程竞争
        std::atomic<page_id_t> next_page_id_;
        //占个位
        std::future<void> *flush_log_f_;

        int num_flushes_;
        int num_writes_;
        bool flush_log_;




    };
}



#endif //LEISQL_DISK_MANAGER_H
