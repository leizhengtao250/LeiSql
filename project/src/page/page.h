//
// Created by lei on 2019/12/22.
//

#ifndef LEISQL_PAGE_H
#define LEISQL_PAGE_H

#include <string.h>
#include "../common/config.h"
namespace leisql{
    /**
     * Page is the basic unit of storge within the database system.
     *
     */
    class Page{
        friend class BufferPoolManager;
    public :
        Page(){
        ResetMemory();//构造函数在初始时就构造出4096字节的空间 作为页面 这样可对应磁盘页面大小
    };
        ~Page(){};
        //disable copy
        Page(Page &page)= delete;
        Page &operator=(const Page &)=delete;

        /**
         * 给页面数据赋值
         * @param data
         */
        inline void setData( char *data){
            for (int i = 0; i < PAGE_SIZE ; ++i) {
                data_[i]=data[i];
            }
        }


        /**
         * @return 返回的是一个page的内容
         */
        inline char * GetData(){
            return data_;
        }

        /**
         * @return 返回当前的page_id;
         */
        inline page_id_t GetPageId(){
            return page_id_;
        }

        /**
         * 返回的是一个pin count
         * pin 指的是如果当前页面正在使用，则为pin，不能对其删除操作
         */
        inline int GetPinCount(){
            return pin_count_;
        }

        /**
         * 如果此页面被修改，返回true，否则返回false
         */
        inline bool isDirty(){
            return is_dirty_;
        }
        /**
         * 返回the page LSN
         * LSN称为日志的逻辑序列号(log sequence number)
         * 根据LSN，可以获取到几个有用的信息：
            1.数据页的版本信息。
            2.写入的日志总量，通过LSN开始号码和结束号码可以计算出写入的日志量。
            3.可知道检查点的位置。
              实际上还可以获得很多隐式的信息。
         */
        inline lsn_t GetLSN(){
            return *reinterpret_cast<lsn_t *>(GetData() + OFFSET_LSN);
        }

        /**
         * memcpy 函数用于 把资源内存（src所指向的内存区域）
         * 拷贝到目标内存（dest所指向的内存区域）；
         * 拷贝多少个？有一个size变量控制
         * void *memcpy(void *dest, void *src, unsigned int count);
         * @param lsn
         */
        inline void SetLSN(lsn_t lsn){
            memcpy(GetData()+OFFSET_LSN,&lsn, sizeof(lsn_t));
        }



    protected:
        static constexpr size_t OFFSET_PAGE_START=0;//constexpr 变量必须在编译时进行初始化。所有 constexpr 变量均为常量，因此必须使用常量表达式初始化。
        static constexpr size_t OFFSET_LSN = 4 ;
    private:
        /**
         * memset 函数是内存赋值函数，用来给某一块内存空间进行赋值的；包含在<string.h>头文件中,
         * 可以用它对一片内存空间逐字节进行初始化；原型为 ：
            void *memset(void *s, int v, size_t n);
            这里s可以是数组名，也可以是指向某一内在空间的指针；
            v为要填充的值；
            n为要填充的字节数；
            开辟一块内存，给此内存赋值为PAGE_SIZE 个 OFFSET_PAGE_START。
         */
        inline void ResetMemory(){
            memset(data_,OFFSET_PAGE_START,PAGE_SIZE);
        }
        char data_[PAGE_SIZE]{};

        /**
         *  当前的page_id 是 默认为 -1 ；
         */
        page_id_t page_id_=INVALID_PAGE_ID;

        /**
         *  有几个应用当前正在使用此页面
         */
        int pin_count_ = 0;

        /**
         * 如果页面是dirty 返回是true。
         * It is different from its corresponding page on disk.
         */
        bool is_dirty_=false;

    };
}
#endif //LEISQL_PAGE_H
