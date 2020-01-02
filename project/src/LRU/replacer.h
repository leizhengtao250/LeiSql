//
// Created by lei on 2019/12/22.
//

#ifndef LEISQL_REPLACER_H
#define LEISQL_REPLACER_H

#include <cstdlib>

namespace leisql{
    template <typename T>
    class replacer {
    public:
        replacer(){};
        virtual ~replacer(){}
        virtual void Insert(const T &value)=0;
        virtual bool Victim(T &value)=0;
        virtual bool Erase(const T &value)=0;
        virtual size_t Size()=0;
    };
}



#endif //LEISQL_REPLACER_H
