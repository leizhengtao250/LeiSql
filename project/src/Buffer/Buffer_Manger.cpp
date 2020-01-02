//
// Created by lei on 2019/12/28.
//

#include "Buffer_Manger.h"
#include "../ClockReplace/ClockReplacer.h"
/**
 * 设置缓存池管理的目的就是允许客户端
 * 1.增加删除在磁盘上的页面
 * 2.读磁盘上的页面到缓存池中并且pin它，或者在缓存池中解除pin
 */
namespace leisql{
    BufferPoolManager::BufferPoolManager(size_t pool_size,DiskManager *diskManager,LogManager *logManager)
    :pool_size_(pool_size),diskManager_(diskManager),logManager_(logManager){
        pages_ = new Page[pool_size_];
        replacer_= new ClockReplacer(pool_size);
        //initially,every page is in the free list
        //clock_replace针对已经分配好的页面
        //free_list 是还没有分配的页面
        // id 是从0 开始的.因为disk的page编号也是从0开始的
        for(size_t i= 0;i<pool_size;i++){
            free_list_.emplace_back(static_cast<int>(i));
        }
    }

    BufferPoolManager::~BufferPoolManager(){
            delete [] pages_;
            delete [] replacer_;
    }

    /**
     *
     * @param pageId
     * @return
     */
    Page * BufferPoolManager::FetchPageImpl(page_id_t pageId){
    // 1.     Search the page table for the requested page (P).
    // 1.1    If P exists, pin it and return it immediately.
    // 1.2    If P does not exist, find a replacement page (R) from either the free list or the replacer.
    //        Note that pages are always found from the free list first.
    // 2.     If R is dirty, write it back to the disk.
    // 3.     Delete R from the page table and insert P.
    // 4.     Update P's metadata, read in the page content from disk, and then return a pointer to P.
        if(page_table_.find(pageId) != page_table_.end()){
        frame_id_t  frameId = page_table_[pageId];
        replacer_->Pin(frameId);
        ++(pages_+frameId)->pin_count_;
        return pages_+frameId;
        }else{
        /**
         * 内存中没有页面，要从磁盘中调入page
         * 新加入的page无非面临两种情况
         * 1.缓冲池有空位,从free list中拿一个位置,即第一个位置里的数字对应的页面数组里位置肯定为空
         * （每次从缓存池中删除一个元素 对应位置的frame_id 加到freelist的头部，这要每次从头部取得一定是位置空的）
         * 2.缓存池没有空位，调用replace
         */
        //1.
            if(replacer_->Size() <pool_size_){
                frame_id_t frameId = free_list_.front();
                page_table_.insert({pageId,frameId});
                char *page_data = nullptr;
                diskManager_->ReadPage(pageId,page_data);
                pages_[frameId].setData(page_data);
                ++pages_[frameId].pin_count_;
                return pages_+frameId;
            }else{
                frame_id_t *frameId= nullptr;
                replacer_->Victim(frameId);
                page_table_.insert({pageId,*frameId});
                char *page_data = nullptr;
                diskManager_->ReadPage(pageId,page_data);
                pages_[*frameId].setData(page_data);
                ++pages_[*frameId].pin_count_;
                return pages_+ (*frameId);
                 }
            }
        }

    /**
     * unpin the target page from the buffer pool
     * @param pageId id of page to be unpinned
     * @param is_dirty true if the page should be marked as dirty.false otherwise.
     * @return false if the page pin count is <=0;before this call,true others.
     */

    bool BufferPoolManager::UnpinPageImpl(page_id_t pageId, bool is_dirty) {
        frame_id_t frameId = page_table_[pageId];
        if(pages_[frameId].pin_count_<=0){
            return false;
        }
        replacer_->Unpin(frameId);
        is_dirty = pages_[frameId].is_dirty_;
        return true;
    }

    /**
     * 写到磁盘上的时候 pin ，并不一定要退出缓存池，将继续存在
     * 只有从内存中删除才可能彻底释放内存
     * @param pageId
     * @return
     */
    bool BufferPoolManager::FlushPageImpl(leisql::page_id_t pageId) {
         if(pageId == INVALID_PAGE_ID){
             return false;
         }

         if(page_table_.find(pageId)==page_table_.end()){
             return false;
         }
         /**
          * 这一个过程 buffer_manger显示调用页面，然后写入页面，页面的pin_count先加1，再减1.
          */
         frame_id_t frameId = page_table_[pageId];
         diskManager_->WritePage(pageId,pages_[frameId].GetData());
         //is_dirty=true的过程不是在这里修改的
         pages_[frameId].is_dirty_= false;//页面算是未修改
    }

    /** Deletes a page from the buffer pool.
     * 判断一个页面是否能删除
     * 1.它是否在pin，页面dirty
     * 2.是否存在
    * @param page_id id of page to be deleted
    * @return false if the page exists but could not be deleted, true if the page didn't exist or deletion succeeded
     *  // 0.   Make sure you call DiskManager::AllocatePage!
        // 1.   If all the pages in the buffer pool are pinned, return nullptr.
        // 2.   Pick a victim page P from either the free list or the replacer. Always pick from the free list first.
        // 3.   Update P's metadata, zero out memory and add P to the page table.
        // 4.   Set the page ID output parameter. Return a pointer to P.
    */

    bool BufferPoolManager::DeletePageImpl(leisql::page_id_t pageId) {
         if(page_table_.find(pageId)==page_table_.end()){
             return false;//此页面不在内存中
         }else{//在内存中
             frame_id_t frameId = page_table_[pageId];
             if(pages_[frameId].is_dirty_ || pages_[frameId].pin_count_>0){
                 return false;
             }else{
                 /**
                  * 删除页面步骤
                  * 1.clockReplacer中删除
                  * 2.缓存池中要删除
                  *    2.1 pages_数组中要删除
                  *    2.2 free_list也要删除
                  *    2.3 page_table_也要删除。
                  */
                 replacer_->Pin(frameId);//从clockReplacer中去除
                 pages_[frameId].setData(nullptr);
                 free_list_.remove_if(frameId);
                 free_list_.push_front(frameId);//空的位置编号放在开头
                 page_table_.erase(pageId);

             }
         }
    }

    /**
     *  // 0.   Make sure you call DiskManager::DeallocatePage!
        // 1.   Search the page table for the requested page (P).
        // 1.   If P does not exist, return true.
        // 2.   If P exists, but has a non-zero pin-count, return false. Someone is using the page.
        // 3.   Otherwise, P can be deleted. Remove P from the page table, reset its metadata and return it to the free list.
     */
    void BufferPoolManager::FlushAllPageImpl() {


    }





}