/*************************************************************************
 > File Name: Node.h
 > Author: Winter
 > Created Time: 2022年05月03日 星期二 10时07分23秒
 ************************************************************************/
#ifndef _NODE_H_
#define _NODE_H_

#include <iostream>
#include <cmath>
#include <cstring>
#include <mutex>
#include <fstream>
#include <cstdlib>

#define STORE_FILE "store/dumpFile"

// 临界段的互斥锁
std::mutex mtx;
std::string delimiter = ":";

/**
 *	节点类
 */

// 使用模板来实现节点
template<typename K, typename V>
class Node {
public:
	Node(){};                  // 无参构造
	Node(K k, V v, int);       // 有参构造
	~Node();                   // 析构函数
	K get_key() const;         // 获得key
	V get_value() const;       // 获得value
	void set_value(V);         // 设置value

	Node<K, V>** forward;
	/*
	forward是一个指针数组，即是数组名，也是数组首地址
	存储的类型是Node<K, V>*
	每个Node<K, V>*代表一层，指向该层的下一个Node节点
	forward[0]存储的是第0层的指针，以此类推
	每一层通过forward连接起来
	通过forward[i]控制层数，通过node = node->foward[i]在每一层移动
	*/
	int node_level;

private:
	K key;
	V value;
};

// 有参构造的实现
template<typename K, typename V>
Node<K, V>::Node(const K k, const V v, int level) {
	this->key = k;
	this->value = v;
	this->node_level = level;
	// 创建forward数组
	this->forward = new Node<K, V>*[level + 1];
	// 将申请的空间置空
	memset(this->forward, 0, sizeof(Node<K, V>*) * (level + 1));
}

// 析构函数
template<typename K, typename V>
Node<K, V>::~Node() {
	delete []forward;
}

// 获得key
template<typename K, typename V>
K Node<K, V>::get_key() const {
	return this->key;
}

// 获得value
template<typename K, typename V>
V Node<K, V>::get_value() const {
	return this->value;
}

// 设置key和value
template<typename K, typename V>
void Node<K, V>::set_value(V value) {
	this->value = value;
}


/************************************************************************************/

template<typename K, typename V>
class SkipList {
public:
	SkipList(int max_level);                 // 构造函数，初始化最大层
	~SkipList();                             // 析构函数
	int get_random_level();                  // 随机获取一个层数，不简单
	Node<K, V>* create_node(K, V, int);      // 创建一个新节点
	int size();                              // 获取跳表大小

	// 下面是插入节点、显示跳表、查找元素、删除元素。是精髓
	int insert_element(K, V);                // 插入(K, V)
	void dispaly_list();                     // 显示跳表
	bool search_element(K);                  // 查找元素
	void delete_element(K);                  // 删除元素

	// 文件操作
	void dump_file();                        // 导出数据
	void load_file();                        // 加载数据


private:
	// 从string中获得key和value
	void get_key_value_from_string(const std::string& str, std::string* key, std::string* value);

	// 判断字符串是否合法
	bool is_valid_string(const std::string& str);

private:
	int max_level_;             // 跳表的最高层
	int skip_list_level_;       // 跳表的当前层
	Node<K, V>* header_;        // 跳表的头指针，不存储数据
	std::ofstream file_writer_; // 文件写操作
	std::ifstream file_reader_; // 文件读操作
	int element_count_;         // 跳表个数
};

// 先实现旁枝末节
/***************************************************************************************/
// 判断字符串是否合法
template<typename K, typename V>
bool SkipList<K, V>::is_valid_string(const std::string& str) {
	if (str.empty()) {
		return false;
	}
	// :只能出现在string中间
	if (str.find(delimiter) == std::string::npos) {
		return false;
	}
	return true;
}

// 从string中获得key和value
template<typename K, typename V>
void SkipList<K, V>::get_key_value_from_string(const std::string& str, std::string* key, std::string* value) {
	if (!is_valid_string(str)) {
		return;
	}
	*key = str.substr(0, str.find(delimiter));
	*value = str.substr(str.find(delimiter) + 1, str.length());
}

/*****************************************************************************************/
// 构造函数
template<typename K, typename V>
SkipList<K, V>::SkipList(int max_level) {
	this->max_level_ = max_level;
	this->skip_list_level_ = 0;
	this->element_count_ = 0;

	K k;
	V v;
	// 创建一个不存数据的头节点
	this->header_ = new Node<K, V>(k, v, max_level_);
}

// 析构函数
template<typename K, typename V>
SkipList<K, V>::~SkipList() {
	if (file_writer_.is_open()) {
		file_writer_.close();
	}
	if (file_reader_.is_open()) {
		file_reader_.close();
	}
	delete header_;
}

// 创建一个新节点
template<typename K, typename V>
Node<K, V>* SkipList<K, V>::create_node(const K key, const V value, int level) {
	Node<K, V>* node = new Node<K, V>(key, value, level);
	return node;
}

// 得到随机层数
template<typename K, typename V>
int SkipList<K, V>::get_random_level() {
	int k = 1;
	// 没有调用随机数种子，rand()每次的值都是1,即k一直都是2
	while (rand() % 2) {
		k++;
	}
	k = (k < max_level_) ? k : max_level_;
	// 返回min(2, max_level_);
	return k;
}

// 插入元素
template<typename K, typename V>
int SkipList<K, V>::insert_element(const K key, const V value) {
	// 加锁
	mtx.lock();                     		// 保持数据的一致性
	Node<K, V>* current = this->header_;    // 保存指向头节点的指针[不存数据]

	Node<K, V>* update[max_level_ + 1];
	memset(update, 0, sizeof(Node<K, V>*) * (max_level_ + 1));

	/*
	 *	这个数组是重点， 因为是插入数据，所以需要保存插入位置前一个节点
	 */

	// 从本节点的最高号层开始遍历
	// 如果current的下一个节点不为空[同层] && current的下一个节点的keyu小于要插入的key
	// current在本层一直后移
	for (int i = skip_list_level_; i >= 0; i--) {
		while (current->forward[i] != nullptr && current->forward[i]->get_key() < key) {
			current = current->forward[i];
		}
		// 此时current是第一个大于要插入key的节点
		// 通过i遍历。控制每层
		update[i] = current;
	}
	current = current->forward[0];               // 此时current就是要插入的位置

	// key已经有了，返回1
	if (current != nullptr && current->get_key() == key) {
		std::cout << "key:" << key << ",exists" << std::endl;
		mtx.unlock();
		return 1;
	}

	// key不存在，创建
	if (current == nullptr || current->get_key() != key) {
		// 为节点生成随机级别
		int random_level = get_random_level();

		// 如果随机级别大于跳表当前的级别，使用指向头节点的指针初始化update，即高层的前驱节点是头节点
		if (random_level > skip_list_level_) {
			for (int i = skip_list_level_ + 1; i < random_level + 1; i++) {
				update[i] = header_;
			}
			skip_list_level_ = random_level;
		}

		// 使用随机级别创建节点
		Node<K, V>* insert_node = create_node(key, value, random_level);

		// 插入节点
		for (int i = 0; i <= random_level; i++) {
			// 链表插入动作
			insert_node->forward[i] = update[i]->forward[i];
			update[i]->forward[i] = insert_node;
		}
		std::cout << "Successfully inserted key:" << key << ", value:" << value << std::endl;
		element_count_++;
	}
	mtx.unlock();
	return 0;
}

// 显示跳表
template<typename K, typename V>
void SkipList<K, V>::dispaly_list() {
	// 总的来说就是：从最底层开始遍历，遍历完一层，再往上遍历。每一层通过forward链接起来。
	std::cout << "\n****************Skip List****************" << "\n";
	for (int i = 0; i <= skip_list_level_; i++) {
		Node<K, V>* node = this->header_->forward[i];
		std::cout << "Level:" << i <<":";
		while (node != nullptr) {
			std::cout << node->get_key() << ":" << node->get_value() <<";";
			node = node->forward[i];
		}
		std::cout << std::endl;
	}
}

// 导出数据
template<typename K, typename V>
void SkipList<K, V>::dump_file() {
	file_writer_.open(STORE_FILE);
	std::cout << "dump file-------------------------" << std::endl;
	// 导出数据，从最底层开始，因为底层最全
	Node<K, V>* node = this->header_->forward[0];

	while (node != nullptr) {
		file_writer_ << node->get_key() << ":" << node->get_value() << "\n";
		std::cout << node->get_key() << ":" << node->get_value() << ";\n";
		node = node->forward[0];
	}
	file_writer_.flush();
	file_writer_.close();
	return;
}

// 加载数据
template<typename K, typename V>
void SkipList<K, V>::load_file() {
	file_reader_.open(STORE_FILE);
	std::cout << "load file-------------------------" << std::endl;
	std::string line;
	std::string* key = new std::string();
	std::string* value = new std::string();

	while (getline(file_reader_, line)) {
		get_key_value_from_string(line, key, value);
		if (key->empty() || value->empty()) {
			continue;
		}

		insert_element(*key, *value);
		std::cout << "key:" << *key << ",value:" << *value << std::endl;
	}
	file_reader_.close();
}

// 获得元素个数
template<typename K, typename V>
int SkipList<K, V>::size() {
	return element_count_;
}

// 删除元素，这里类比插入元素
template<typename K, typename V>
void SkipList<K, V>::delete_element(K key) {
	mtx.lock();
	Node<K, V>* current = this->header_;
	Node<K, V>* update[max_level_ + 1];
	memset(update, 0, sizeof(Node<K, V>*) * (max_level_ + 1));

	for (int i = skip_list_level_; i >= 0; i--) {
		while (current->forward[i] != nullptr && current->forward[i]->get_key() < key) {
			current = current->forward[i];
		}
		update[i] = current;
	}

	current = current->forward[0];
	if (current != NULL && current->get_key() == key) {
        for (int i = 0; i <= skip_list_level_; i++) {
            if (update[i]->forward[i] != current) {
				break;
			}
			// 删除节点
            update[i]->forward[i] = current->forward[i];
        }
		// 这里需要好好斟酌
        while (skip_list_level_ > 0 && header_->forward[skip_list_level_] == nullptr) {
            skip_list_level_ --; 
        }
		delete current;         // 释放内存
        std::cout << "Successfully deleted key "<< key << std::endl;
        element_count_ --;
    }
    mtx.unlock();
    return;
}

// 查找元素
template<typename K, typename V>
bool SkipList<K, V>::search_element(K key) {
	std::cout << "search_element-----------------" << std::endl;
    Node<K, V> *current = header_;

    for (int i = skip_list_level_; i >= 0; i--) {
        while (current->forward[i] && current->forward[i]->get_key() < key) {
            current = current->forward[i];
        }
    }

    current = current->forward[0];

    if (current != nullptr && current->get_key() == key) {
        std::cout << "Found key: " << key << ", value: " << current->get_value() << std::endl;
        return true;
    }

    std::cout << "Not Found Key:" << key << std::endl;
    return false;
}

#endif