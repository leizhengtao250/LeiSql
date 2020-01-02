//
// Created by lei on 2019/12/22.
//

#include <assert.h>
#include "LRU.h"
/**
 * 整个就是双向链表的增删改查
 */
namespace leisql{
    template <typename T> //T为 page_id ，LRU 操作的是page
    LRU<T>::LRU():size_(0) {
        head_=std::make_unique<node>();
        tail=head_.get();
    }

    template <typename T>
    LRU<T>::~LRU()=default;

    /**
     * 插入要做的是把page_id 放到维护链表中
     * 先把page_id 放到 node 里 ，再把node放到链表中
     * 查询的话可以先根据page_id在map中寻找node的地址
     * @tparam T
     * @param value
     */
    template <typename T>
    void LRU<T>::Insert(const T &value ) {
        auto it = table_.find(value);
        if(it == table_.end()){//没找到
            tail->next=std::make_unique<node>(value,tail);
            tail=tail->next.get();
            table_.emplace(value,tail);
            ++size_;
        }//ToDo 若插入的page_id 已经在链表中else
    }




    template <typename T>
    bool LRU<T>::Victim(T & value) {

        if(size_== 0){
            assert(head_.get()==tail);//如果括号中是true，那么就会执行assert
            return false;
        }

        value = head_->data;
        head_->next=std::move(head_->next->next);
        if(head_->next !=nullptr){//若等于空，本来有2个元素，剔除一个，还剩1个，若不等于空，至少还剩两个
            head_->next->pre=head_.get();
        }
        //移除了链表，同时map中也要擦除
        table_.erase(value);

        if(size_==0){
            tail=head_.get();
        }

        return true;

    }


    template <typename T>
    bool LRU<T>::Erase(const T & value) {
        auto it = table_.find(value);
        if(it==table_.end()){
            return false;//不在维护的链表中
        }
        if(it->second == head_){
           node * p = head_;
           head_=head_->next;
           head_->pre= nullptr;
            delete p;
        }else if(it->second == tail){
            node *q = tail;
            tail=tail->pre;
            tail->next= nullptr;
            delete q;
        }else{
            node * r = it->second;
            r->pre->next=r->next;
            r->next->pre=r->pre;
            delete r;
        }

        table_.erase(value);
        if(--size_==0){
            head_==tail;
        }
        return true;

    }

    template <typename T>
    size_t LRU<T>::Size() {
        return size_;
    }


}