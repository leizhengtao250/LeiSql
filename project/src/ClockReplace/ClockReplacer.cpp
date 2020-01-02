//
// Created by lei on 2019/12/31.
//

#include "ClockReplacer.h"
#include "../common/logger.h"
namespace leisql {
    ClockReplacer::ClockReplacer(size_t num_pages):curr_pages(0){
        /**
         * 构造循环链表
         */
        head_ = std::make_shared<node>();
        tail_ = head_;
        tail_->next = head_;
        curr_p=head_;
    }
    /**
     * 如果内存中的一个page被pin，那么它不能参与置换了。也就是
     * 不能存在与clockReplacer中,在有进程访问它之前存在于clock中，那么就得从
     * clockReplacer中去除。
     * @param frameId
     */
    void ClockReplacer::Pin(frame_id_t frameId) {
         node *pre = nullptr;
         node *p = head_.get();
         pre = p;
         while(p!=nullptr){
             if(p->frameId==frameId){
                 node *tmp = p ;
                 pre->next=p->next;
                 delete tmp;
             }
             pre = p;
             p = p->next.get();
         }
         --curr_pages;
    }

    /**
     * 如果一个页面暂时在缓存池中没有进程访问它，
     * 即pin_count=0那么应当先放到clockReplacer中
     * 一个页面先被访问然后不访问是就会被unpin到clockReplacer中，
     * 作为候选被替换的页面
     * 这里不用考虑最大长度
     * @param frameId
     */
    void ClockReplacer::Unpin(frame_id_t frameId) {
        std::shared_ptr<node> tmp;
        tmp = std::make_shared<node>();
        tmp->frameId = frameId;
        tmp->reference=1;
        tmp->next=head_;
        tail_->next = tmp;
        tail_=tail_->next;
        ++curr_pages;
    }

    /**
     * pin 和 victim 函数很像，pin不管在clockReplacer中满载还是不满载都可删除
     * 而victim只有在clockReplacer中满载才会找出替换页面
     * 1.缓存池中页面数量满载
     * 2.从disk中调入了新的page到缓存池中
     * 3.新的页面要在内存中寻找一个frame去存储
     * 4,当这个页面没有进程访问的时候，要将其unpin到clockReplacer中
     * 5.需要替换那个frame，需要把那个frame 通过clockReplacer 找出来并且有 *
     * @param frameId
     * @return 将 cr中frame地址返回出来，就可以知道缓存池中哪个位置可以替换页面了
     */

   bool ClockReplacer::Victim(frame_id_t  *frameId) {
       std::shared_ptr<node> p = curr_p;
       while(p != curr_p){
           if(p->reference==1){
               p->reference=0;
           }
           p=p->next;
       }

        while(p != curr_p){
            if(p->reference==0){
                p->reference=1;
                frameId= &p->frameId;
                return true;
            }
            p=p->next;
        }
        return false;
    }




    size_t ClockReplacer::Size() {
        return curr_pages;
    }


}