//
// Created by lei on 2019/12/28.
//

#ifndef LEISQL_REPLACE_H
#define LEISQL_REPLACE_H


#include "../common/config.h"

namespace leisql{
    /**
     * replacer is an abstract class that tracks page usage.
     */
    class Replacer {
    public:
        Replacer()= default;
        virtual ~Replacer()= default;

         /**
          * remove the victim frame as define by the replacement policy
          * @param frameId
          * @return true if a victim frame was found,false otherwise
          */
         virtual bool Victim(frame_id_t *frameId)=0;


         /**
          * pins a frame,indicating that it should not be vicitimized until it is unpinned.
          * @param frameId
          */
         virtual void Pin(frame_id_t frameId)=0;

         /**
          * Unpin a frame,indicating that it can now be victimized.
          * @param frameId
          */
         virtual void Unpin(frame_id_t frameId)=0;

         /**
          * the number of elements in the replacer
          * @return
          */
         virtual size_t Size()=0;



    };

}


#endif //LEISQL_REPLACE_H
