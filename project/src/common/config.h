//
// Created by lei on 2019/12/22.
//

#ifndef LEISQL_CONFIG_H
#define LEISQL_CONFIG_H

#include <chrono>
#include <atomic>

namespace leisql{
static constexpr int PAGE_SIZE=4096;//每个页面的尺寸为4096byte
static constexpr int INVALID_PAGE_ID=-1;
static constexpr int INVALID_LSN=-1;
static constexpr int BUFFER_POOL_SIZE=10;
static constexpr int LOG_BUFFER_SIZE=((BUFFER_POOL_SIZE+1)*PAGE_SIZE);
static constexpr int BUCKET_SIZE=50;
/**
 * 为了保证平台的通用性，程序中尽量不要使用long数据库型。
 * 可以使用固定大小的数据类型宏定义
 */
using page_id_t = int32_t ;
using lsn_t = int32_t;
using frame_id_t = int32_t ;



}



#endif //LEISQL_CONFIG_H
