#include "engine.h"

NetEngine::NetEngine()
{
    // 初始化对应的执行任务pid为-1
    this -> mCurrent_pid = -1;
}

void NetEngine::work(char** cmd, const std::string& redirect)
{
    // 创建孙子进程  
    pid_t pid = fork();
    if (pid == -1) // 如果fork失败  
    {  
        perror("fork子进程失败");
        throw std::runtime_error(__FILE__ + std::string(": ") + std::to_string(__LINE__) + ": fork子进程失败");
    }
    else if(pid == 0) // 孙子进程
    {
        // 设置重定向
        int fd = open(redirect.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0644);
        if(fd < 0)
        {
            std::cerr << redirect << std::endl;
            throw std::runtime_error(__FILE__ + std::string(": ") + std::to_string(__LINE__) + ": 无法打开指定的重定向文件");
        }
        //printf("fd = %d\n",fd);
        dup2(fd,STDOUT_FILENO);
        // 孙子进程变身成对应程序执行对应指令
        execvp(cmd[0], cmd);  

        // 如果execlp返回，说明发生了错误  
        std::cerr << cmd[0] << std::endl;
        perror("execvp函数执行失败");
        
        throw std::runtime_error(__FILE__ + std::string(": ") + std::to_string(__LINE__) + ": execvp函数执行失败");
    }
    else
    {
        // 儿子进程代码  
        // 记录子进程的PID  
        //debug("Child PID: %d\n", pid);
        this->mCurrent_pid = pid;
    }
}

void NetEngine::join()
{
    pid_t result = waitpid(this->mCurrent_pid, &(this->mStatus), 0);
    // std::cout << BOLDGREEN << __FILE__ << __LINE__  << RESET << std::endl;
    // 当时程序出了个问题，上面这一行代码死活没法执行（waitpid之后进程直接死掉），
    // 我排查了好长时间，最后取消监听17这个中断信号之后程序才正常执行了下去
    // 当我看到上面的绿字重新出现在屏幕上的时候还是感觉很治愈的（
    // 毕竟排查了6个小时的错误……
    // 现在程序发布了这一行注释掉还是多少有些不舍和不放心，虽然软件已经能正常工作了……
    
    if (result == -1)
    {
        // 错误处理
        perror("waitpid函数执行失败");
        throw std::runtime_error(__FILE__ + std::string(": ") + std::to_string(__LINE__) + ": waitpid函数执行失败");
    }
 
    if (WIFEXITED(this -> mStatus))
    {
        //printf("Child exited with code %d\n", WEXITSTATUS(status));
    }
    else
    {
        //printf("Child terminated abnormally\n");
    }
    
}

void NetEngine::suicide()
{
    if(this -> mCurrent_pid != -1)
    {
        kill(mCurrent_pid, SIGINT); // 通知当前孙子死一死
    }
}