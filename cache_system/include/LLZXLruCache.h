#pragma once

#include <cmath>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

#include "LLZXCachePolicy.h"

namespace LLZXCache
{

template<typename Key, typename Value> class LLZXLruCache;

// 定义LRU缓冲节点，包括一个Key和一个值，然后记录访问次数，初始化时次数为1
template<typename Key, typename Value>
class LruNode
{
private:
	Key key_;
	Value value_;
	size_t accessCount_;	//访问次数
	std::weak_ptr<LruNode<Key, Value>> prev_;
	std::shared_ptr<LruNode<Key, Value>> next_;

public:
	LruNode(Key key, Value value)
		: key_(key)
		, value_(value)
		, accessCount_(1)
	{}

	//节点访问器
	Key getKey() const {return key_;}
	Value getValue() const {return value_;}
	void setValue(const Value& value) {value_ = value;}
	size_t getAccessCount() const{return accessCount_;}
	void incrementAccessCount() { ++accessCount_; }

	friend class LLZXLruCache<Key, Value>;
};


template<typename Key, typename Value>
class LLZXLruCache: public LLZXCachePolicy<Key, Value>
{
	using LruNodeType = LruNode<Key, Value>;
    using NodePtr = std::shared_ptr<LruNodeType>;
    using NodeMap = std::unordered_map<Key, NodePtr>;

	// 添加缓存
	void put(Key key, Value value) override
	{
		if (capacity_ <= 0) return;

		std::lock_guard<std::mutex> lock(mutex_);
		auto it = nodeMap_.find(key);
		if(it != nodeMap_end()) {
			updateExistingNode(it->second, value);
			return;
		}

		addNewNode(key, value);
	}

	bool get(Key key, Value& value) override
	{
		std::lock_guard<std::mutex> lock(mutex_);
		auto it = nodeMap_.find(key);
		if(it !=  nodeMap_.end())
		{
			updateExistingNode(it->second, value);
			value = it->second->getValue();
			return true;
		}
		return false;
	}

	Value get(Key key) override
    {
        Value value{};
        // memset(&value, 0, sizeof(value));   // memset 是按字节设置内存的，对于复杂类型（如 string）使用 memset 可能会破坏对象的内部结构
        get(key, value);
        return value;
    }

	// 删除指定元素
	void remove(Key key)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		auto it = nodeMap_.find(key);
		if(it != nodeMap_.end())
		{
			removeNode(it->second);
            nodeMap_.erase(it);
		}
	}
private:
	void initializeList()
	{
		//创建首尾虚拟节点
		dummyHead_ = std::make_shared<LruNodeType>(Key(), Value());
		dummyTail_ = std::make_shared<LruNodeType>(Key(), Value());
		dummyHead_->next_ = dummyTail_;
		dummyTail_->next_ = dummyHead_;
	}

	void updateExistingNode(NodePtr node, const Value& value)
	{
		node->setValue(value);
		moveToMostRecent(node);
	}
	
	void addNewNode(const Key& key, const Value& value)
	{
		if(nodeMap_.size() >= capacity_)
		{
			//大于容量,驱逐
			evictLeastRecent();
		}

		NodePtr newNode = std::make_shared<LruNodeType>(key, value);
		insertNode(newNode);
		nodeMap_[key] = newNode;
	}

	// 移动节点到最新位置
	void moveToMostRecent(NodePtr node) {
		removeNode(node);
		insertNode(node);
	}

	void removeNode(NodePtr node) 
	{
		/*
		expired() 方法返回一个布尔值：
		true：表示 weak_ptr 指向的对象已经被销毁（引用计数为0）
		false：表示 weak_ptr 指向的对象仍然存在
		*/
		if(!node->prev_.expired() && node->next_)
		{
			auto prev = node->prev_.lock();
			prev->next_ = node->next_;
			node->next_->prev_ = prev;
			node->next_ = nullptr;// 清空next_指针，彻底断开连接
		}
	}

	// 从尾部插入
	void insertNode(NodePtr node) {
		node->next_ = dummyTail_;
		node->prev_ = dummyTail_->prev_;
		dummyTail_->prev_.lock()->next_= node;
		dummyTail_->prev_ = node;
		//weak_ptr 不能直接访问成员，需要先调用 lock() 获取对应的 shared_ptr，如果 lock() 返回空，说明对象已被销毁
	}
	// 驱逐最近最少访问
	void evictLeastRecent()
	{
		NodePtr leastRecent = dummyHead_->next_;
		removeNode(leastRecent);
		nodeMap_.erase(leastRecent->getKey());
	}

//可以优化成使用list链表，更加现代
private:
    int           capacity_; // 缓存容量
    NodeMap       nodeMap_; // key -> Node 
    std::mutex    mutex_;
    NodePtr       dummyHead_; // 虚拟头结点
    NodePtr       dummyTail_;
};

// LRU优化：Lru-k版本，通过继承的方式进行再优化
template<typename Key, typename Value>
class LLZXLruKCache : public LLZXLruCache<Key, Value>
{
public:
	LLZXLruKCache(int capacity, int historyCapacity, int k)
		: LLZXLruCache<Key, Value>(capacity)
		, historyList_(std::make_unique<LLZXLruCache<Key, size_t>>(historyCapacity))
		, k_(k)
	{}

	Vaule get(Key key)
	{
		// 首先尝试从主缓存获取数据
		Value value{};
		bool inMainCache = LLZXLruCache<Key, Value>::get(key, value);

		// 获取并更新访问历史计数
		size_t historyCount = historyList_->get(key);
		historyCount++;
		historyList_->put(key, historyCount);

		// 如果在主缓存中，直接返回
		if (inMainCache)
		{
			return value;
		}

		// 不在，检查是否达到了k次访问
		if(historyCount >= k_)
		{
			auto it = historyValueMap_.find(key);
			if(it != historyValueMap_.end())
			{
				Value storedValue = it->second;
				historyList_->remove(key);
				historyValueMap_.erase(it);

				LLZXLruCache<Key, Value>::put(key, storedValue);
				return storedValue;
			}
			//没有找到，返回默认值
		}

		return value; // 默认值
	}

	void put(Key key, Value value)
	{
		Value exitingValue{};
		bool inMainCache = LLZXLruCache<Key, Value>::get(key, exitingValue);
		if(inMainCache)
		{
			LLZXLruCache<Key, Value>::put(key, value);
			return;
		}

		size_t historyCount = historyList_->get(key);
		historyCount++;
		historyList_->put(key, historyCount);

		// 保存值到历史记录映射，供后续get操作使用
		historyValueMap_[key] = value;
		
		if(historyCount >= k_)
		{
			historyList_->remove(key);
			historyValueMap_.erase(key);
			LLZXLruCache<Key, Value>::put(key, value);
		}
	}
private:
	int k_;//进入缓存的门槛
	std::unique_ptr<LLZXLruCache<Key, size_t>> historyList_; // 记录访问历史的缓存
	std::unordered_map<Key, Value> historyValueMap_; // 记录存储未达到k次访问的数值
};


}