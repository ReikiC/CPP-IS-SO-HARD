#include <iostream>
#include <stack>
#include <mutex>
#include <stdexcept>
#include <string>

// ============================================
// C++ 版本的 ObjectPool
// ============================================

template<typename T>
class ObjectPool {
private:
    // 用 std::stack 存储对象指针
    std::stack<T*> pool;
    
    // 互斥锁（相当于 C# 的 lock）
    std::mutex mutex;
    
    // 容量
    const int capacity;
    
public:
    // 构造函数
    explicit ObjectPool(int cap) : capacity(cap) {
        // capacity 是只读的，必须用初始化列表赋值
    }
    
    // 析构函数（C# 没有，C++ 需要手动清理）
    ~ObjectPool() {
        // 清空池子中的所有对象
        while (!pool.empty()) {
            T* obj = pool.top();
            pool.pop();
            delete obj;  // 释放内存
        }
    }
    
    // 获取容量
    int GetCapacity() const {
        return capacity;
    }
    
    // 获取当前对象数量
    int GetCount() {
        std::lock_guard<std::mutex> lock(mutex);  // 自动加锁/解锁
        return pool.size();
    }
    
    // 从池中取对象（相当于 C# 的 Pop）
    T* Pop() {
        std::lock_guard<std::mutex> lock(mutex);  // 加锁
        
        if (pool.empty()) {
            throw std::runtime_error("池是空的！");
        }
        
        T* obj = pool.top();  // 获取栈顶对象
        pool.pop();           // 移除栈顶
        return obj;
    }
    
    // 把对象放回池（相当于 C# 的 Push）
    void Push(T* item) {
        if (item == nullptr) {
            throw std::invalid_argument("不能把 nullptr 放进对象池");
        }
        
        std::lock_guard<std::mutex> lock(mutex);  // 加锁
        pool.push(item);
    }
};

// ============================================
// 测试代码
// ============================================

// 简单的测试类
class Connection {
private:
    int id;
    
public:
    Connection(int id) : id(id) {
        std::cout << "创建连接 " << id << std::endl;
    }
    
    ~Connection() {
        std::cout << "销毁连接 " << id << std::endl;
    }
    
    void Use() {
        std::cout << "使用连接 " << id << std::endl;
    }
    
    int GetId() const { return id; }
};

int main() {
    std::cout << "=== C++ ObjectPool 测试 ===\n" << std::endl;
    
    // 1. 创建对象池
    ObjectPool<Connection> pool(10);
    std::cout << "创建对象池，容量：" << pool.GetCapacity() << std::endl;
    std::cout << "当前对象数：" << pool.GetCount() << "\n" << std::endl;
    
    // 2. 往池里放对象
    std::cout << "放入 3 个连接..." << std::endl;
    pool.Push(new Connection(1));
    pool.Push(new Connection(2));
    pool.Push(new Connection(3));
    std::cout << "当前对象数：" << pool.GetCount() << "\n" << std::endl;
    
    // 3. 从池里取对象
    std::cout << "取出对象..." << std::endl;
    Connection* conn1 = pool.Pop();
    std::cout << "取出连接 " << conn1->GetId() << std::endl;
    std::cout << "当前对象数：" << pool.GetCount() << "\n" << std::endl;
    
    Connection* conn2 = pool.Pop();
    std::cout << "取出连接 " << conn2->GetId() << std::endl;
    std::cout << "当前对象数：" << pool.GetCount() << "\n" << std::endl;
    
    // 4. 使用对象
    std::cout << "使用对象..." << std::endl;
    conn1->Use();
    
    // 5. 把对象还回池子
    std::cout << "\n把对象还回去..." << std::endl;
    pool.Push(conn1);
    std::cout << "当前对象数：" << pool.GetCount() << "\n" << std::endl;
    
    // 6. 重复使用（再借出来）
    std::cout << "再次借出..." << std::endl;
    Connection* conn3 = pool.Pop();
    std::cout << "取出连接 " << conn3->GetId() << std::endl;
    std::cout << "是同一个对象吗？" << (conn3 == conn1 ? "是" : "否") << "\n" << std::endl;
    
    // 7. 清理（C++ 需要手动，C# 由 GC 处理）
    std::cout << "手动删除借出的对象..." << std::endl;
    delete conn2;
    delete conn3;
    
    std::cout << "\n 测试完成！" << std::endl;
    std::cout << "（池子里剩余的对象会在析构函数中自动清理）\n" << std::endl;

    getchar();
    
    // pool 的析构函数会自动调用，清理剩余对象
    return 0;
}

/* 
编译命令：
g++ -std=c++11 -o objectpool objectpool.cpp
./objectpool

预期输出：
=== C++ ObjectPool 测试 ===

创建对象池，容量：10
当前对象数：0

放入 3 个连接...
创建连接 1
创建连接 2
创建连接 3
当前对象数：3

取出对象...
取出连接 3
当前对象数：2

取出连接 2
当前对象数：1

使用对象...
使用连接 3

把对象还回去...
当前对象数：2

再次借出...
取出连接 3
是同一个对象吗？是

手动删除借出的对象...
销毁连接 2
销毁连接 3

✅ 测试完成！
（池子里剩余的对象会在析构函数中自动清理）

销毁连接 1
*/