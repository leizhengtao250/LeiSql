//
// Created by lei on 2019/12/29.
//

#include "Disk_Manager.h"
#include "../common/logger.h"
#include <string.h>
#include <assert.h>
namespace leisql {
    static char *buffer_used;

    /**
     * 打开或者创建一个数据库文件
     * @param db_file
     */
    DiskManager::DiskManager(const std::string &db_file)
    :file_name_(db_file),next_page_id_(0),num_flushes_(0),num_writes_(0),flush_log_(false),flush_log_f_(nullptr) {
        /**
         * 从逻辑上来讲，size() 成员函数似乎应该返回整形数值，或是无符号整数。但事实上，size 操作返回的是 string::size_type 类型的值。
         * 虽然我们不知道 string::size_type 的确切类型，但可以知道它是 unsigned 型。对于任意一种给定的数据类型，它的 unsigned 型所能表示的最大正数值比对应的 signed 型要大一倍。这个事实表明 size_type 存储的 string 长度是 int 所能存储的两倍。
         * 主要目的是随着机器不同变化而变化
         * find返回的是在字符串中第一次发现 . 的索引,If not found, returns npos.
         */
        std::string::size_type n = file_name_.find(".");
        if (n == std::string::npos) {
            LOG_DEBUG("wrong file format");
            return;
        }
        /**
         * 例如db文件名 a.lei
         * log的名称为 a.log
         */
        log_name_ = file_name_.substr(0, n) + ".log";

        /**
         * std::ios::binary |std::ios::in|std::ios::app|std::ios::out 这几个有一个成立就可以
         *  //ios::app：　　　以追加的方式打开文件
            //ios::ate：　　　文件打开后定位到文件尾，ios:app就包含有此属性
            //ios::binary： 　  以二进制方式打开文件，缺省的方式是文本方式。
            //ios::in：　　　  文件以输入方式打开
            //ios::out：　　　文件以输出方式打开
            //ios::nocreate： 不建立文件，所以文件不存在时打开失败　
            //ios::noreplace：不覆盖文件，所以打开文件时如果文件存在失败
            //ios::trunc：　　如果文件存在，把文件长度设为0
            可以用“或”把以上属性连接起来，如ios::out|ios::binary
            | 代表或者
         */

        log_io_.open(log_name_, std::ios::binary | std::ios::in | std::ios::app | std::ios::out);
        //directory or file does not exist
        /**
         * 状态标志符的验证(Verification of state flags)
         * 新的成员函数叫做eof ，它是ifstream 从类 ios 中继承过来的，当到达文件末尾时返回true 。
         * bad()如果在读写过程中出错，返回 true 。例如：当我们要对一个不是打开为写状态的文件进行写入时，或者我们要写入的设备没有剩余空间的时候。
           fail()除了与bad() 同样的情况下会返回 true 以外，加上格式错误时也返回true ，例如当想要读入一个整数，而获得了一个字母的时候。
           eof()如果读文件到达文件末尾，返回true。
           good() 这是最通用的：如果调用以上任何一个函数返回true 的话，此函数返回 false 。
           要想重置以上成员函数所检查的状态标志，你可以使用成员函数clear()，没有参数。
         */
        if (!log_io_.is_open()) {
            log_io_.clear();

            //create a new file
            log_io_.open(log_name_, std::ios::binary | std::ios::in | std::ios::app | std::ios::out);
            log_io_.close();

            //reopen with original mode
            log_io_.open(log_name_, std::ios::binary | std::ios::in | std::ios::app | std::ios::out);
        }
        /**
         * 打开数据库文件
         */
        db_io_.open(db_file,std::ios::binary | std::ios::in | std::ios::app | std::ios::out);

        if(!db_io_.is_open()){
            db_io_.clear();

            db_io_.open(db_file,std::ios::binary | std::ios::trunc | std::ios::out);
            db_io_.close();

            db_io_.open(db_file,std::ios::binary | std::ios::in | std::ios::out);
        }
        buffer_used= nullptr;
    }

        /**
         * 关闭所有文件流
         */
        void DiskManager::Shutdown() {
            db_io_.close();
            log_io_.close();
        }

        /**
         * 将有关页面写到磁盘中
         */
        void DiskManager::WritePage(page_id_t pageId, const char * page_data) {
            /**
             * 一个文件存在磁盘上，是由多个page组成
             * 假设page编号从0开始，一个page大小为 PAGE_SIZE
             * page_id * PAGE_SIZE = offset
             * offset 就是 一个page 开始的位置，将游标设定在这个位置，即从这个位置开始写入
             */
            size_t offset = static_cast<size_t>(pageId) * PAGE_SIZE;
            //set write cursor to offset
            //从 offset位置开始写
            num_writes_+=1;
            db_io_.seekp(offset);
            db_io_.write(page_data,PAGE_SIZE);
            //如果出现写入错误
            if(db_io_.bad()){
                LOG_DEBUG("IO error while writing");
                return;//结束此次写入
            }
            /**
             * 3.缓冲区的类型
            缓冲区 分为三种类型：全缓冲、行缓冲和不带缓冲。
            1）全缓冲
                在这种情况下，当填满标准I/O缓存后才进行实际I/O操作。全缓冲的典型代表是对磁盘文件的读写。
            2）行缓冲
                在这种情况下，当在输入和输出中遇到换行符时，执行真正的I/O操作。这时，我们输入的字符先存放在缓冲区，等按下回车键换行时才进行实际的I/O操作。典型代表是键盘输入数据。
            3）不带缓冲
                也就是不进行缓冲，标准出错情况stderr是典型代表，这使得出错信息可以直接尽快地显示出来。

             */
            db_io_.flush();//清空缓冲区的数据，即强行将缓冲区数据写到磁盘中
        }

        void DiskManager::ReadPage(page_id_t pageId, char * page_data) {
            /**
             * 读一个page 也是先定位
             * offset 单位为 B，一个字节
             */
             size_t offset = static_cast<size_t>(pageId) * PAGE_SIZE;
             if(offset>GetFileSize(file_name_)){
                LOG_DEBUG("read error ");

             }else{
                 db_io_.seekp(offset);
                 db_io_.read(page_data,PAGE_SIZE);
                 //以用成员函数 int gcount();来取得实际读取的字符数
                 //page 并不是全部存满了 数据
                 int read_count = db_io_.gcount();
                 if(read_count<PAGE_SIZE){
                     LOG_DEBUG("Read less than a PAGE");
                     //将没用数据的地方都用0填满
                     memset(page_data+read_count,0,PAGE_SIZE-read_count);
                 }
             }
        }


    /**
* Allocate new page (operations like create index/table)
* For now just keep an increasing counter
*/
        page_id_t DiskManager::AllocatePages() {
            return next_page_id_++;
        }


/**
 * Deallocate page (operations like drop index/table)
 * Need bitmap in header page for tracking pages
 * This does not actually need to do anything for now.
 */
        void DiskManager::DeallocatePage(leisql::page_id_t pageId) {

        }





}































