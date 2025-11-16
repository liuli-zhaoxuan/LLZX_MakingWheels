#pragma once

#include <iostream>
#include <unordered_map>
#include <list>
#include <mutex>
#include <memory>

#include "LLZXCachePolicy.h"

namespace LLZXCache
{

//前向声明
template<typename Key, typename Value> class LRUCache;

template<typename Key, typename Value>
class LruNode
{
private:
    Key key_;
    Value value_;
    size_t accessCount_; //访问次数
    
public:
    LruNode(Key key, Value value) : key_(key), value_(value), accessCount_(1) {}

    // 提供必要的访问接口
    Key getKey() const { return key_; }
    Value getValue() const { return value_; }
    void setValue(Value value) { value_ = value; }
    size_t getAccessCount() const { return accessCount_; }
    void incrementAccessCount() { accessCount_++; }

    friend class LRUCache<Key, Value>;
}


}
