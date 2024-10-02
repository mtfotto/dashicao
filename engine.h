#ifndef DASHICAO_ENGINE_H
#define DASHICAO_ENGINE_H

#include "utils.h"

// 网络引擎类NetEngine
// 设计思想：
// 若想要引擎实例工作，需要实例化一个引擎对象并且调用work方法
// 在工作的时候，实例会fork()一个子进程并命令子进程工作，
// 父进程中的引擎实例会存储子进程的pid
// 在启动工作后，需要调用实例的join方法等待子进程完成工作
// 函数suicide酱紫方法用于和中断函数绑定
// 当检测到Ctrl + C的时候，实例的suicide函数将会被调用
// 用于终止工作的子进程
// 所有的引擎实例都是用完即弃的，
// 如果想要重复干活需要重新实例化一个新的引擎
class NetEngine
{
protected:
    // 启动进程的pid
    int mCurrent_pid;
    // 启动的子进程的状态
    int mStatus;

public:

    // 初始化一个网络引擎
    // 初始化的时候可以指定工作的类型
    NetEngine();

    // 工作函数
    void work(char** cmd, const std::string& redirect = "/dev/null");

    // 阻塞当前进程，等待子进程完工
    void join();

    // 酱紫函数
    // 对工作的子进程发送Ctrl + C信号
    // 可以在中断函数里面调用
    void suicide();
};

#endif