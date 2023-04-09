#include <iostream>
#include <chrono>
#include <thread>
#include <functional>

class Defer {
  public:
    Defer(std::function<void()> fn) : fn_(fn) {}
    ~Defer() { fn_(); }

  private:
    std::function<void()> fn_;
};

// 使用defer来计算函数的执行时间
void test() {
    auto start = std::chrono::system_clock::now();
    Defer f([&start](){
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "in this func time elapsed: " << elapsed_seconds.count() << "s"
                  << std::endl;}
    );

    for(int i=0;i<INT32_MAX;i++);
    std::this_thread::sleep_for(std::chrono::seconds(3));
}

int main() {
        // 获取当前时间
        auto start = std::chrono::system_clock::now();
        std::time_t start_time = std::chrono::system_clock::to_time_t(start);
        std::cout << "Current time: " << std::ctime(&start_time) << std::endl;

        // 等待一段时间
        std::this_thread::sleep_for(std::chrono::seconds(2));

        // 获取时间差值
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Time elapsed: " << elapsed_seconds.count() << "s"
                  << std::endl;

        test();

        return 0;
}
