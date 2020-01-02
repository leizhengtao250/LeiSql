//
// Created by lei on 2019/12/18.
//

#include <assert.h>
#include "Extendible_hash.h"
#include <map>
#include <cmath>
#include <iostream>

namespace leisql {
    template<typename K, typename V>
    ExtendibleHash<K, V>::ExtendibleHash(size_t size):
            bucket_size_(size), bucket_count_(0), depth(0), pair_count(0) {
        /**
         * emplace_back能就地通过参数构造对象，不需要拷贝或者移动内存，
         * 相比push_back能更好地避免内存的拷贝与移动，
         * 使容器插入元素的性能得到进一步提升。
         */
        directory_.emplace_back(new Bucket(0, 0));
        /**
         * bucket initial
         */
        bucket_count_ = 1;
    }

    /**
     * 利用c++ 的hash 来计算 HashKey的哈希值，
     * 尽量做到讲key 转换为 size_t 类型
     * 并且均匀分布
     */
    template<typename K, typename V>
    size_t ExtendibleHash<K, V>::HashKey(const K &key) {
        return std::hash<K>()(key);
    }

    /**
     * 根据提供的id 来获取 bucket_id对应的bucket中的 深度
     * @tparam K
     * @tparam V
     * @param bucket_id
     * @return
     */
    template<typename K, typename V>
    int ExtendibleHash<K, V>::GetLocalDepth(size_t bucket_id) const {
        assert(bucket_id >= 0 && bucket_id < static_cast<int>(directory_.size()));//bucket_id 不满足条件就终止程序、
        if (directory_[bucket_id]) {
            return directory_[bucket_id]->depth; //vectory 存的是指向bucket的共享指针
        }
        return -1;

    }

    /**
     * 这个是返回全部深度之和
     * @tparam K
     * @tparam V
     * @return
     */
    template<typename K, typename V>
    int ExtendibleHash<K, V>::GetGlobalDepth() const {
        return depth;//ToDo 后面要做到线程保护
    }

    /**
     * 返回总的bucket个数
     * @tparam K
     * @tparam V
     * @return
     */
    template<typename K, typename V>
    int ExtendibleHash<K, V>::GetNumBuckets() const {
//        return static_cast<int>(directory_.size());
        return bucket_count_;
    }

/**
 * 根据提供的k值得到bucket_id
 * @tparam K
 * @tparam V
 * @param key
 * @return
 */
    template<typename K, typename V>
    size_t ExtendibleHash<K, V>::Bucket_Index(const K &key,int cur_depth) {
//        size_t key_temp = HashKey(key);
//        return key_temp%(bucket_count_);
        return HashKey(key)>>(maxSizt_t-cur_depth) & (static_cast<size_t>(pow(2,cur_depth)-1));//根据当前深度来求bucket_id
    }

    /**
     * 根据k值去寻找Value ,再把value赋给v
     * @tparam K
     * @tparam V
     * @param k
     * @param v
     * @return
     */
    template<typename K, typename V>
    bool ExtendibleHash<K, V>::Find(const K &k, V &v) {
        size_t index = Bucket_Index(k,depth);
        if (directory_[index]) {
            auto bucket = directory_[index];
            /**
             * **find()**查找key，找到返回该key的迭代器，否则返回map的尾部(end())
             * 没找到就返回 end（）
             * 如果相等，就是没找到 不等就是找到
             */
            if (bucket->items.find(k) != bucket->items.end()) {
                v = bucket->items[k];
                return true;
            }
        }
        return false;
    }

    /**
     * 根据k 移除对应的元素
     * @tparam K
     * @tparam V
     * @param k
     * @return
     */
    template<typename K, typename V>
    bool ExtendibleHash<K, V>::Remove(const K &k) {
        size_t index = Bucket_Index(k,depth);
        size_t tmp = 0;
        if (directory_[index]) {
            auto bucket = directory_[index];
            if (bucket->items.find(k) != bucket->items.end()) {
                /**
                 * map在删除key-value时，有可能删除不成功
                 */
                tmp = bucket->items.erase(k);//删除指定key的元素，删除成功返回1，否则返回0
                pair_count -= tmp;//成功减1，不成功减0
                return tmp != 0;
            }
        }
        return false;
    }

    /**
     * 插入键值对k,v，这个算法没考虑到bucket溢出，拆分bucket。
     * @tparam K
     * @tparam V
     * @param k
     * @param v
     * @return
     */

    template<typename K, typename V>
    bool ExtendibleHash<K,V>::Insert2(const K & key,const V &value){
        //计算 bucket_id
        size_t bucket_id = Bucket_Index(key,depth);

        /*
        因为ExtendibleHash在初始化的时候会在directory中增加一个指向bucket的指针
        默认size() 为1，不为空
        为空是因为directory_ resize()之后才会得到空指针
        */
        if(directory_[bucket_id]==nullptr){
            directory_[bucket_id] = std::make_shared<Bucket>(bucket_id, depth);
            ++bucket_count_;
        }
        /**
          * 如果bucket 存在，且map 中存在,修改
          */
        auto cur_bucket = directory_[bucket_id];
        if(cur_bucket->items.find(key) != cur_bucket->items.end()){
            cur_bucket->items[key]=value;
        }else{//bucket存在，且map中不存在，插入
            /**
              插入一条记录，bucket 满
            **/
            if (cur_bucket->items.size()==bucket_size_-1)
            {
                cur_bucket->items.insert({key,value});
                auto new_bucket = split(cur_bucket);
                directory_.resize( static_cast<int> (pow(2,depth)));

                directory_[cur_bucket->id] = cur_bucket;
                directory_[new_bucket->id]=new_bucket;
                ++pair_count;
                return true;
            }else{
                /**
                  插入一条记录，bucket 未满
                **/
                cur_bucket->items.insert({key,value});
                ++pair_count;
                return true;
            }
        }
        return false;
    }


    /**
     * 插入kv ，要考虑到bucket的容量,即map的容量
     * 插入过程：1.先依据（key，value）的可以计算成hashK
     *         2.hashK 经过移位操作得到 offset
     *         3.看看vector[offset]中有没有Bucket
     *         4.如果没有Bucket，新建一个，Bucket_id = offset;
     *         5.如果有BUcket，看key值是否存过，若存过则修改，若没有，插入Bucket中的map
     *         6.如果插入溢出，则split
     *
     */
    template<typename K, typename V>
    void ExtendibleHash<K, V>::Insert(const K &key, const V &value) {
        std::lock_guard<std::mutex> lock(mutex_);
        size_t bucket_id =Bucket_Index(key,depth);
        assert(bucket_id < directory_.size());
        if (directory_[bucket_id] == nullptr) {//
            directory_[bucket_id] = std::make_shared<Bucket>(bucket_id, depth);
            ++bucket_count_;
        }


        auto bucket = directory_[bucket_id];
        // already in bucket, override
        if (bucket->items.find(key) != bucket->items.end()) {//如果key存在于bucket中的一个map中
            bucket->items[key] = value;  //修改
            return;
        }
        // insert to target bucket
        bucket->items.insert({key, value});
        ++pair_count;
        // may need split & redistribute bucket
        if (bucket->items.size() > bucket_size_ && !bucket->overflow) {//并且目标bucket即将溢出
            // record original bucket index and local depth
            auto old_index = bucket->id;
            auto old_depth = bucket->depth;
            std::shared_ptr<Bucket> new_bucket = split(bucket);
            // if overflow restore original local depth
            if (new_bucket == nullptr) {
                bucket->depth = old_depth;
                return;
            }

            // rearrange pointers, may need increase global depth
            if (bucket->depth > depth) {
                auto size = directory_.size();
                auto factor = (1 << (bucket->depth - depth));
                // global depth always greater equal than local depth
                depth = bucket->depth;
                directory_.resize(directory_.size() * factor);
                // fill original/new bucket in directory
                directory_[bucket->id] = bucket;
                directory_[new_bucket->id] = new_bucket;
                // update to right index: for buckets not the split point
                for (size_t i = 0; i < size; ++i) {
                    if (directory_[i]) {
                        // clear stale relation
                        if (i < directory_[i]->id ||
                            // important filter: not prefix any more
                            ((i & ((1 << directory_[i]->depth) - 1)) != directory_[i]->id)) {
                            directory_[i].reset();
                        } else {
                            auto step = 1 << directory_[i]->depth;
                            for (size_t j = i + step; j < directory_.size(); j += step) {
                                directory_[j] = directory_[i];
                            }
                        }
                    }
                }
            } else {
                // reset directory entries which points to the same bucket before split
                for (size_t i = old_index; i < directory_.size(); i += (1 << old_depth)) {
                    directory_[i].reset();
                }
                // add all two new buckets to directory
                directory_[bucket->id] = bucket;
                directory_[new_bucket->id] = new_bucket;
                auto step = 1 << bucket->depth;
                for (size_t i = bucket->id + step; i < directory_.size(); i += step) {
                    directory_[i] = bucket;
                }
                for (size_t i = new_bucket->id + step; i < directory_.size(); i += step) {
                    directory_[i] = new_bucket;
                }
            }
        }
    }

    /**
     * 对即将满载的bucket拆分,下文的key是经过hash之后得到的hash码。
     * 拆分过程：1.满载了
     *         2.例如之前的bucket_id=0b11,那么这里（bucket）存储的数据的key（64位）的前面2位为 11
     *         3.开辟一个新的bucket。深度加。1那么bucket分为old_bucket，new_bucket
     *         4.将以前bucket的数据重新分配。old_bucket的id为前三位110，new_bucket的id为前三位111.将每条数据的key前三位取出来，对应一下就可以分配了
     *         5.map里插入的是最先给的key-value，没有经过hash
     */
    template<typename K, typename V>
    std::shared_ptr<typename ExtendibleHash<K, V>::Bucket> ExtendibleHash<K, V>::split(std::shared_ptr<Bucket> &b) {
        /**
         * 前提是b之前在b->depth的前提下已经满了
         * 重新再开辟一个bucket
         */
        auto res = std::make_shared<Bucket>(0, b->depth);//new Bucket(id=0,depth)
        depth++;//总的深度加1
        while (res->items.empty()) {
            ++b->depth;
            ++res->depth;
            b->id=b->id<<1;//在原始bucket中的id右边添一个0
            //对原先b中的map的每条记录遍历
            for (auto it = b->items.begin(); it != b->items.end(); it++){
                //if(Bucket_Index(it->first))-> 这样做不行，因为b->depth深度改变了 it->first 指的是map中的key值
                size_t Bucket_id = Bucket_Index(it->first,b->depth);
                if (Bucket_id != b->id) {
                    res->items.insert(*it);
                    res->id = Bucket_id; //HASH 算法
                    it = b->items.erase(it);//把原先的bucket中的map对应的kv擦除，腾出空间
                }
            }
            /**
             * 如果说b经过remove操作，得到一个空的集合，那么会将res与b交换，res为空，b应该为满
             * 这一部分有必要吗
             * 有，如果b是空的，那么说明这些数据都满足b未split之前的条件，那么这些数据也满足res的条件，依据统计学的原理
             * 所以接下来的数据极有可能去往res，所以再把res和b交换一下。
             */

            if (b->items.empty()) {
                b->items.swap(res->items);//swap将两个map中kv全部交换
                b->id = res->id;
            }

            //size_t 在64位系统为 64位，那么 sizeof(size_t)=8;
            //因为移位操作最大为64为或者32位，看系统
            //此时没办法在new出新的bucket，返回空指针
            if (b->depth == sizeof(size_t) * 8) {
                b->overflow = true;
                return nullptr;
            }
            ++bucket_count_;//总的bucket数量加1
            return res;
        }

    }


    template<typename K, typename V>
    std::shared_ptr<typename ExtendibleHash<K, V>::Bucket> ExtendibleHash<K, V>::split2(std::shared_ptr<Bucket> &b){
            auto res = std::make_shared<Bucket>(0,b->depth);
            ++depth;
            ++b->depth;
            b->id=(b->id<<1);

            for(auto it = b->items.begin();it != b->items.end();++it){
                size_t bucket_id = Bucket_Index(it->first,depth);

                if(bucket_id != b->id){
                    res->items.insert(*it);
                    res->id = bucket_id ;
                    it = b->items.erase(it);
                    res->depth = b->depth;
                }
            }
            if(depth > maxSizt_t){
                b->overflow==true;
                return nullptr;
            }

            ++bucket_count_;
            return res;



    }

}