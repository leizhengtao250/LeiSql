//
// Created by lei on 2019/12/28.
//

#ifndef LEISQL_BUFFER_MANGER_H
#define LEISQL_BUFFER_MANGER_H

#include <cstddef>
#include "../DiskManger/Disk_Manager.h"
#include "../Log/log_manager.h"
#include "../page/page.h"
#include "../LRU/LRU.h"
#include "../ClockReplace/Replace.h"
#include <list>
namespace leisql{
    class BufferPoolManager {
    public:
        //枚举类比枚举好
        enum class CallbackType{BEFORE,AFTER};
        //这是一个函数指针
        //只要函数满足这样的参数列表 即可调用此指针指向目标函数
        using bufferpool_callback_fn=void(*)(enum CallbackType,const page_id_t pageId);
        /**
         *
         * 创建一个新的BufferPoolManager
         * @param pool_size 缓冲池的尺寸
         * @param diskManager
         * @param logManager
         */
        BufferPoolManager(size_t pool_size,DiskManager *diskManager,LogManager *logManager);

        ~BufferPoolManager();

        /**
         *
         * @param pageId
         * @param callback 可自行填写，这里只是一个函数指针作为参数，具体编写可放到外面
         * @return
         */
       Page *FetchPage(page_id_t pageId,bufferpool_callback_fn callback= nullptr){
            GradingCallback(callback,CallbackType::BEFORE,pageId);
            auto *result = FetchPageImpl(pageId);
            GradingCallback(callback,CallbackType::AFTER,pageId);
            return result;
       }




    private:

        /**
         * 如果不为空，调用回调函数
         * @param callback
         * @param callbackType
         * @param pageId
         */
        void GradingCallback(bufferpool_callback_fn callback,CallbackType callbackType,page_id_t pageId){
           if(callback != nullptr){
               callback(callbackType,pageId);
           }
       }

       Page *FetchPageImpl(page_id_t pageId);

        /**
         *
            * Unpin the target page from the buffer pool.
            * @param page_id id of page to be unpinned
           * @param is_dirty true if the page should be marked as dirty, false otherwise
           * @return false if the page pin count is <= 0 before this call, true otherwise
         */
       bool UnpinPageImpl(page_id_t pageId,bool is_dirty);


        /**
         * Flushes the target page to disk.
         * @param page_id id of page to be flushed, cannot be INVALID_PAGE_ID
         * @return false if the page could not be found in the page table, true otherwise
         */
       bool FlushPageImpl(page_id_t pageId);

        /**
  * Deletes a page from the buffer pool.
  * @param page_id id of page to be deleted
  * @return false if the page exists but could not be deleted, true if the page didn't exist or deletion succeeded
  */
       bool DeletePageImpl(page_id_t pageId);

        /**
   * Flushes all the pages in the buffer pool to disk.
   */
       void FlushAllPageImpl();

       Page * pages_;

       size_t pool_size_; //缓存池中page的数量

       DiskManager *diskManager_;

       LogManager *logManager_;

       Replacer *replacer_;

       std::list<frame_id_t> free_list_;

       std::unordered_map<page_id_t,frame_id_t> page_table_;


    };
}



#endif //LEISQL_BUFFER_MANGER_H
