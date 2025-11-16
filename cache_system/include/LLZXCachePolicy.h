#pragma once
//LLZX的缓存
namespace LLZXCache
{

template<typename Key, typename Value>
class LLZXCachePolicy {
public:
    virtual ~LLZXCachePolicy() = default;
    
    // 添加缓存接口
    virtual void put(Key key, Value value) = 0;

    // key是传入参数，访问到的值以传出参数的形式返回 | 访问成功返回true
    virtual bool get(Key key, Value& value) = 0;

};

} //namespace LLZXCache