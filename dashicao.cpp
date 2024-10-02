// 思路：./dashicao爸爸进程生一堆孩子进程
// 孩子进程生孙子进程，孩子进程负责监督孙子进程
// 孙子进程负责执行网络请求操作执行完就死
// 孙子死了之后孩子重新生孙子循环往复
// 直到用户发送Ctrl + C指令

// 导库
#include "utils.h"                   // 库函数和宏定义
#include "read_config.h"             // 读取config和解析命令行
#include "engine.h"                  // 导入所有网络请求引擎

// 进程管理
int process_count;                   // 进程数量
int* pool;                           // 主进程的进程池，里面存孩子们的pid
auto current_children = 0;           // 记录当前孩子们的数量，针对孩子没生完就停止程序的情况
int* shm_p;                          // 统计子进程一共干了多少活
int shm_id;                          // 共享内存ID

// 子进程自己用的东西
NetEngine* gEngine = nullptr;         // 子进程工作引擎指针，新建一个骨灰盒然后里面什么也不放
std::string cmd_str = "";             // 要执行的命令字符串，包含未展开的宏
char** cmd = nullptr;                 // 最终要执行的命令

// 日志相关
#define warn gLogger->warning()
#define debug gLogger->debug()
#define info gLogger->info()
#define notice gLogger->notice()
#define error gLogger->error()
#define critic gLogger->critic()
Log::Config::Vector gConfigList;
Log::Logger* gLogger = nullptr;

// 设置
Config* gConfig = nullptr;
std::string logger_dir = "";
auto logger_level = 0;
std::string redirect = "/dev/null";                // 重定向到的文件
unsigned int seed = 0;
bool loop = false;
using std::cout;
using std::endl;

Log::Log::Level getLevel(int input)
{
    if(input < 0)
        return Log::Log::eDebug;
    switch(input)
    {
    case 0:
        return Log::Log::eDebug;
        break;
    case 1:
        return Log::Log::eInfo;
        break;
    case 2:
        return Log::Log::eNotice;
        break;
    case 3:
        return Log::Log::eWarning;
        break;
    case 4:
        return Log::Log::eError;
        break;
    case 5:
        return Log::Log::eCritic;
        break;
    }
    return Log::Log::eCritic;
}

 
// 自定义的mkdirp函数，用于递归创建目录  
bool mkdirp(const char *path) {  
    struct stat st = {0};  
    if (stat(path, &st) == 0) {  
        // 如果路径已存在，并且是一个目录，则成功  
        if (S_ISDIR(st.st_mode)) {  
            return true;  
        }  
        // 如果路径已存在但不是目录，返回失败  
        return false;  
    }  
  
    // 尝试找到最后一个'/'  
    char *dir = strdup(path);  
    char *p = dir + strlen(dir) - 1;  
    while (p > dir && *p != '/') p--;  
  
    // 如果p指向的是目录的开始，说明path是一个根目录或者已经是一个目录路径了  
    if (p == dir) {  
        p++;  
    } else {  
        *p = 0; // 截断字符串到最后一个'/'  
        if (!mkdirp(dir)) {  
            free(dir);  
            return false;  
        }  
        *p = '/'; // 恢复'/'  
    }  
  
    // 创建目录  
    return mkdir(path, 0755) == 0;  
}  

// 检查文件是否存在，如果不存在则创建文件（包括递归创建目录）
bool createFileIfNotExists(const char *filepath)
{  
    struct stat buffer;  
    if (stat(filepath, &buffer) == 0) {  
        // 文件已存在  
        return true;  
    }  
  
    // 尝试创建目录（如果filepath包含目录的话）  
    char *dirname = strdup(filepath);  
    char *p = dirname + strlen(dirname) - 1;  
    while (p > dirname && *p != '/') p--;  
    if (p > dirname) {  
        *p = 0; // 截断为目录部分  
        if (!mkdirp(dirname)) {  
            std::cerr << "Failed to create directory for " << filepath << std::endl;  
            free(dirname);  
            return false;  
        }  
        *p = '/'; // 恢复'/'  
    }  
    free(dirname);  
  
    // 尝试创建文件  
    int fd = open(filepath, O_WRONLY | O_CREAT, 0644);  
    if (fd == -1) {  
        std::cerr << "Failed to create file " << filepath << std::endl;  
        return false;  
    }  
    close(fd);  
    return true;  
} 
bool createFileIfNotExists(std::string filepath)
{
    return createFileIfNotExists(filepath.c_str());
}

// 每个进程在干活之前都要先初始化命令
void init_cmd()
{
    // 设置logger日志路径
    logger_dir = gConfig->replace_all_variable(gConfig->mLogger_dir);
    // 设置logger级别
    logger_level = std::stoi(gConfig->replace_all_variable(gConfig->mLogger_level));
    // 范围裁剪
    if(logger_level < 0)
    {
        logger_level = 0;
    }
    if(logger_level > 5)
    {
        logger_level = 5;
    }
    if(gConfig->replace_all_variable(gConfig->mLogOutputConsole) == "true")
        Log::Config::addOutput(gConfigList, "OutputConsole");
    Log::Config::addOutput(gConfigList, "OutputFile");
    createFileIfNotExists(logger_dir);
    Log::Config::setOption(gConfigList, "filename", logger_dir.c_str());
    Log::Config::setOption(gConfigList, "filename_old", (logger_dir + ".old").c_str());
    Log::Config::setOption(gConfigList, "max_startup_size", "0");
    Log::Config::setOption(gConfigList, "max_size", std::to_string(1 << 16).c_str());
    redirect = gConfig->replace_all_variable(gConfig->mRedirect);
    Log::Manager::setDefaultLevel(getLevel(logger_level));
    try
    {
        // Configure the Log Manager (create the Output objects)
        Log::Manager::configure(gConfigList);
    }
    catch (std::exception& e)
    {
        std::cerr << e.what();
    }
    gLogger = new Log::Logger(("进程" + std::to_string(gConfig->mProcess_id)).c_str());
    info << getpid() << "进程初始化中……";

    // 是否循环工作
    {
        std::string loop_string = gConfig->replace_all_variable(gConfig->mLoop);
        loop = loop_string == "true";
        debug << "是否循环工作：" << loop_string;
    }

    // 准备命令行
    // 按照空格切一下
    auto string_vec = split(cmd_str, " ");
    debug << "当前进程获取到的指令宏为" << cmd_str;
    // 准备好容器
    auto size = string_vec->size();
    short count = 0;
    cmd = new char*[size + 1];
    cmd[size] = NULL;
    std::string preview = "";
    for(auto vars:(*string_vec)) // 遍历
    {
        std::string single_command = gConfig->replace_all_variable(vars); // 单个命令
        preview += single_command + " ";
        cmd[count] = new char[single_command.length() + 1]; // 需要考虑结束符
        strcpy(cmd[count], single_command.c_str()); // cmd[count] = single_command.c_str().copy()
        count++;
    }
    debug << "展开宏之后要执行的指令为" << preview;
    delete string_vec;
}

// 处理后事函数
void stop(int sig)
{
    // 骨灰盒非空
    if(gEngine != nullptr)
    {
        // 引擎死一死
        gEngine -> suicide();
        // 火化
        delete gEngine;
        // 骨灰扬了
        gEngine = nullptr; 
    }

    /* 解除共享内存地质映射。 */
    //shmdt() 与文件的close() 类似，并不会真正清除。
    if (shmdt(shm_p) < 0)
    {
        error << "shmdt()" << "执行失败！";
        perror("shmdt()");
        exit(1);
    }
    info << getpid() << "：结束\n";
    if(gLogger != nullptr)
        delete gLogger;
    Log::Manager::terminate();
    exit(0); // 子进程酱紫
}

// 每个子进程工作函数
void work(int sig)
{
    // 准备指令
    init_cmd();
    info << getpid() << "：初始化完毕，开始阻碍[奋斗]";
    do // 啊啊啊啊 啊啊啊啊啊
    {
        // 新建一个牛马
        gEngine = new NetEngine();
        // 让牛马工作
        gEngine -> work(cmd, redirect);
        // 等牛马干完活
        gEngine -> join();
        // 火化
        delete gEngine; 
        // 骨灰扬了，清空骨灰盒
        gEngine = nullptr;
        // 记录世间曾有一位牛马来过……
        (*shm_p)++;
    } while (loop);
    stop(0);
}

// 停止主进程处理函数
void Get_CtrlC_handler(int sig)
{
    // 酱紫命令说一遍就够了
    signal(sig, SIG_IGN);                         // 忽略多余的酱紫命令
    info << "主进程Ctrl + C，正在命令子进程酱紫...";
    for(auto i = 0; i < current_children; i++)    // 挨个下发酱紫通知
    {
        send(pool[i], STOP);                      // 能不能去死一死
        debug << "命令" << pool[i] << "酱紫";
    }

}

// 主函数，程序的入口点
int main(int argc, char *argv[])
{
    // 解析命令
    const std::string config_path = parse_command(argc, argv); // 解析命令行参数为文件路径
    gConfig = new Config(config_path);                         // 新建一个设置对象并获取地址
    gConfig->mProcess_id = 0;                                  // 父进程编号为0

    // 设置logger
    if(gConfig->replace_all_variable(gConfig->mLogOutputConsole) == "true")
        Log::Config::addOutput(gConfigList, "OutputConsole");
    logger_dir = gConfig->replace_all_variable(gConfig->mLogger_dir);
    Log::Config::addOutput(gConfigList, "OutputFile");
    createFileIfNotExists(logger_dir);
    Log::Config::setOption(gConfigList, "filename", logger_dir.c_str());
    Log::Config::setOption(gConfigList, "filename_old", (logger_dir + ".old").c_str());
    Log::Config::setOption(gConfigList, "max_startup_size",  "0");
    Log::Config::setOption(gConfigList, "max_size", std::to_string(1 << 16).c_str());
    int level = std::stoi(gConfig->replace_all_variable(gConfig->mLogger_level));
    Log::Manager::setDefaultLevel(getLevel(level));
    try
    {
        // Configure the Log Manager (create the Output objects)
        Log::Manager::configure(gConfigList);
    }
    catch (std::exception& e)
    {
        std::cerr << e.what();
    }

    gLogger = new Log::Logger(("进程" + std::to_string(0)).c_str());     // 父进程（产）生（日）志（的容）器

    std::vector<std::string> cmd_pool;                         // 任务池
    // 共享内存
	key_t semkey = ftok(config_path.c_str(), 114);
	if(semkey < 0)  
	{  
		printf("ftok failed\n");  
		exit(EXIT_FAILURE);  
	} 
    /* 创建一个共享内存，创建出来的共享内存权限为0600 */
    shm_id = shmget(semkey, sizeof(int), IPC_CREAT|IPC_EXCL|0600);
    if (shm_id < 0)
    {
        critic << "shmget()" << "执行失败！";
        perror("shmget()");
        exit(1);
    }
    /* 将创建好的共享内存映射进父进程的地址以便访问。 */
    shm_p = (int *)shmat(shm_id, NULL, 0);
    if ((void *)shm_p == (void *)-1)
    {
        error << "shmat()" << "执行失败！";
        perror("shmat()");
        exit(1);
    }
    *shm_p = 0; // 归零

    // 初始化随机
    auto seed = std::stoi(gConfig->replace_all_variable(gConfig->mRandom_seed));
    gConfig->init_random(seed);
    
    // 计算所需要的进程数量
    auto process_num = 0; // 进程数量
    auto num_base = std::stoi(gConfig->replace_all_variable(gConfig->mProcess_num_base)); // 基数
    for(auto cmd: gConfig->mCmds) // 循环每一个命令
    {
        // 每个任务都分配多少进程
        auto each_num = num_base + std::stoi(gConfig->replace_all_variable(std::get<1>(cmd)));
        // 进程总数更新
        process_num += each_num;
        // 任务池更新
        for(auto j = 0; j < each_num; j++)
            cmd_pool.push_back(std::get<0>(cmd));
    }

    info << "初始化中...";
    notice << "请勿关闭程序，否则可能会产生僵尸进程";

    // 使用父进程创建若干个子进程
    pool = new int[process_num];                       // 初始化进程池孩子窝
    int pid = 1;                                       // 一开始的一定爸爸
    for(auto i = 0;i < process_num; i++)               // 计划生育
    {
        pid = fork();                                  // 创建子进程
        if(pid)                                        // 如果当前是父进程
        {
            pool[i] = pid;                             // 获取到的pid放入进程池
            current_children += 1;                     // 记录又生了一个
            cmd_pool.pop_back();                       // 孩子领走了一个任务
        }
        else                                           // 如果是子进程
        {
            gConfig->mProcess_id = i + 1;              // 记录自己的id
            cmd_str = cmd_pool[cmd_pool.size() - 1];   // 领任务
            cmd_pool.clear();                          // 剩下的任务和当前孩子没关系了，直接扔了
            break;                                     // 滚
        }
    }

    if(pid == 0) // 如果是孩子进程
    {
        info << gConfig->mProcess_id << "号进程初始化成功，PID：" << getpid();
        // 监听停止命令
        signal(STOP,stop);
        // 监听启动命令
        signal(START,work);
        while(1) // 等待
        {
            debug << getpid() << "进程开始等待……";
            delete gLogger;                            // 弃用爸爸的生志器
            gConfigList.clear();                       // 清空全局日志设置
            Log::Manager::terminate();                 // 终止当前日志管理器
            sleep(10000);
        }
    }
    if(pid) //是爸爸
    {
        // 爸爸在一开始就要听着什么时候主人要求酱紫
        signal(SIGINT, Get_CtrlC_handler);
        sleep(1); // 等等孩子初始化，不要鸡娃
        notice << "初始化应该完成了，开始干活，随时可停止";
        for(int i = 0; i < current_children; i++)
        {
            send(pool[i], START); // 通知这个孩子开始干活
            debug << "向" << pool[i] << "号进程" << pid << "发送启动指令";
        }
        //debug("print pool:\n");
        //print_pool(pool);
        // 等待所有孩子死掉
        for(int i = 0; i < current_children; i++)
        {
            waitpid(pool[i], 0, 0);
        }
        info << "完成的任务数量：" << (*shm_p);        

        // 清理工作
        if (shmctl(shm_id, IPC_RMID, NULL) < 0) // 删除共享内存
        {
            error << "shmctl()" << "执行失败！";
            perror("shmctl()");
            exit(1);
        }
        info << "程序正常退出";
        if(gLogger != nullptr)
            delete gLogger;
        Log::Manager::terminate();
    }
    return 0;
}