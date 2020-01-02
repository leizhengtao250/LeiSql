//
// Created by lei on 2019/12/31.
//

#ifndef LEISQL_CLOCKREPLACER_H
#define LEISQL_CLOCKREPLACER_H

#include <memory>
#include "Replace.h"
#include "../page/page.h"

namespace leisql{
    class ClockReplacer: public Replacer  {
        struct node{
            node()= default;
            int reference=0;//访问位
            frame_id_t frameId=0;
            std::shared_ptr<node> next;
        };




    public:
        /**
         * 这里页面的最大逻辑（代表页面）个数和缓存池中的页面个数应该相等
         * create a new ClockReplacer
         * @param num_pages the maximum number of pages
         * in the ClockReplace will be required to store
         */
        explicit ClockReplacer(size_t num_pages);

        /**
         * Destroys the ClockReplacer
         */
         ~ClockReplacer() override ;


        bool Victim(frame_id_t *frameId) override ;

        void Pin(frame_id_t frameId) override ;

        void Unpin(frame_id_t frameId) override ;

        size_t Size() override ;

    private:
        size_t curr_pages;//当前的页面数
        std::shared_ptr<node> head_;//链表头结点
        std::shared_ptr<node> tail_;//链表尾结点，方便插入
        std::shared_ptr<node> curr_p;//当前指针


    };
}



#endif //LEISQL_CLOCKREPLACER_H
