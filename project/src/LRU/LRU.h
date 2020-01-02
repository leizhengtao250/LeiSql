//
// Created by lei on 2019/12/22.
//

#ifndef LEISQL_LRU_H
#define LEISQL_LRU_H

#include <memory>
#include <unordered_map>

#include "replacer.h"

namespace leisql{
    template <typename T>
    class LRU :public replacer<T>{
         struct node{
             node()= default;
             explicit node(T d,node *p= nullptr):data(d),pre(p){}
             T data;
             node *pre=nullptr;
             std::unique_ptr<node> next;
         };
    public:
        LRU();
        ~LRU();

        //杜绝复制功能的实现
        LRU(const LRU &)=delete;//复制构造函数删除
        LRU &operator=(const LRU &)=delete;//等于= 赋值 操作删除

        void Insert(const T &);

        bool Victim(T &value);

        bool Erase(const T &value);

        size_t Size();

    private:
        void check();
        /**
         * 头指针
         */
        std::unique_ptr<node> head_;
        /**
         * 尾指针
         */
        node *tail;
        /**
         * 当前维护的page 数量
         */
        size_t size_;
        /**
         * 存值（page_id）和指针（指向node节点的指针）
         */
        std::unordered_map<T,node*> table_;

    };

}



#endif //LEISQL_LRU_H
