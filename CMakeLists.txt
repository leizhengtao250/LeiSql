cmake_minimum_required(VERSION 3.15)
project(LeiSql)

set(CMAKE_CXX_STANDARD 14)

add_executable(LeiSql main.cpp project/src/Hash/Extendible_hash.cpp project/src/Hash/Extendible_hash.h project/src/LRU/LRU.cpp project/src/LRU/LRU.h project/src/LRU/replacer.h project/src/page/page.h project/src/common/config.h project/src/Buffer/Buffer_Manger.cpp project/src/Buffer/Buffer_Manger.h project/src/ClockReplace/Replace.h project/src/DiskManger/Disk_Manager.cpp project/src/DiskManger/Disk_Manager.h project/src/common/logger.h project/src/Log/log_manager.cpp project/src/Log/log_manager.h project/src/ClockReplace/ClockReplacer.cpp project/src/ClockReplace/ClockReplacer.h)

