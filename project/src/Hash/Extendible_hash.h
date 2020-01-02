//
// Created by lei on 2019/12/18.
//

#ifndef LEISQL_EXTENDIBLE_HASH_H
#define LEISQL_EXTENDIBLE_HASH_H

#include <map>
#include <memory>
#include <mutex>
#include <vector>
/**
 * ToDo 1.线程保护
 *      2.溢出问题，怎么解决
 */
namespace leisql{
    template <typename K,typename V>
    class ExtendibleHash{

        /**
         * depth 解释
         * 详细解释看ppt 深度和前面位数绑定
         */

        /**
         * struct 没有指定范围，就是私有的,用户不用去管理bucket
         */
        struct Bucket{
            Bucket() = default;
            explicit Bucket(size_t i,int d):id(i),depth(d){}
            std::map<K,V> items;// 储存的键值对
            bool overflow = false;//是否满载
            size_t id=0; //id of Bucket
            int  depth =1; // local depth counter
        };
    private:
        std::shared_ptr<Bucket> split(std::shared_ptr<Bucket> &);//ToDo
        std::shared_ptr<Bucket> split2(std::shared_ptr<Bucket> &);//ToDo

        mutable  std::mutex mutex_;//to project shared data structure
        const size_t bucket_size_;//bucket size 每个bucket所能存储map键值对的上限
        int bucket_count_;// bucket count
        int depth; //如果溢出，会新增一个bucket，深度加1
        std::vector<std::shared_ptr<Bucket>> directory_;//
        size_t pair_count;//总的map的键值对的个数
        int maxSizt_t = sizeof(size_t)*8;// 移位操作需要

    public:
        explicit ExtendibleHash(size_t size);
        ExtendibleHash(const ExtendibleHash &)=delete;// 删除默认拷贝函数
        ExtendibleHash &operator=(const ExtendibleHash &)=delete;//删除默认赋值函数

        size_t HashKey(const K &key);

        size_t Bucket_Index(const K &,int cur_depth);

        bool Find(const K &k,V &v);

        bool Insert2(const K &k, const V &v);
       void Insert(const K &k, const V &v);

        bool Remove(const K &k);

        int GetGlobalDepth() const;

        int GetLocalDepth(size_t bucket_id) const;

        int GetNumBuckets() const;
        size_t Size()const  {
            return 0;//ToDo
        };
        ~ExtendibleHash(){}
    };



}


#endif //LEISQL_EXTENDIBLE_HASH_H
