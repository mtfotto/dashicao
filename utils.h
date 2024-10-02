#ifndef DASHICAO_UTILS_H
#define DASHICAO_UTILS_H

// 所有要用到的头文件
#include <assert.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <random>
#include <utility>
#include <algorithm>
#include <stdexcept>
#include <tuple>
#include <vector>
#include <regex>

#include <yaml-cpp/yaml.h>
#include <LoggerCpp/LoggerCpp.h>

// 控制台常用颜色
#define RESET "\033[0m"
#define BLACK "\033[30m" /* 黑色 */
#define RED "\033[31m" /* 红色 */
#define GREEN "\033[32m" /* 绿色 */
#define YELLOW "\033[33m" /* 黄色 */
#define BLUE "\033[34m" /* 蓝色 */
#define MAGENTA "\033[35m" /* 洋红 */
#define CYAN "\033[36m" /* 青色 */
#define WHITE "\033[37m" /* 白色 */
#define BOLDBLACK "\033[1m\033[30m" /* 粗黑色 */
#define BOLDRED "\033[1m\033[31m" /* 粗红色 */
#define BOLDGREEN "\033[1m\033[32m" /* 粗绿色 */
#define BOLDYELLOW "\033[1m\033[33m" /* 粗黄色 */
#define BOLDBLUE "\033[1m\033[34m" /* 粗蓝色 */
#define BOLDMAGENTA "\033[1m\033[35m" /* 粗洋红 */
#define BOLDCYAN "\033[1m\033[36m" /* 粗青色 */
#define BOLDWHITE "\033[1m\033[37m" /* 粗白色 */

#define INIT_CONFIG "#####################\n# 钱虎cpp2024版权所有\n# 本文件为大市操项目的配置文件\n#####################\n\n# 配置文件使用的变量，其中：\n# 保留字 #random# 用于产生一个随机的6位数字\n# 保留字 #process_id# 用于代表当前工作的进程编号（0号1号2号等）\n# 所有配置文件中的字符串都可以使用形如#variable#这样的方式\n# 来实现变量的使用（也可以理解为是一种宏替换）\n# 使用变量的时候请小心，因为yaml认为#后面的东西是注释\n# 所以记得及时加引号\n# 程序不会对vars下的宏进行替换\n# 如果一个宏被使用 了但是没有在vars当中被定义，\n# 则该宏只会被简单的去掉两边的#\n# 宏中的空格不会导致一个宏被切分到两个命令参数里面\nvars:\n    engine: \"wget\" # 指定引擎为wget\n    output: \"--output-document=/dev/null\" # 指定输出文件\n    host: \"https://dashichang.work/\" # 大市唱\n    seed: 37 # 使用研究生数字作为随机种子\n    redirect: \"/dev/null\" # 重定向路径\n\n    # 日志 级别宏 不建议动\n    debug: 0\n    info: 1\n    notice: 2\n    warning: 3\n    error: 4\n    critic: 5\n    \n# 是否控制台显示日志\nlogOutputConsole: true\n\n# 所有日志都会存在这个文件里面\nlogger_dir: \"dsc_log/日志.#process_id#.txt\" \n\n# 进程日志级别，填写数字或者下列6个字符串中的一个\n# 可选：   debug, info, notice, warning, error, critic\n# 对应数字：0，    1，   2，     3，      4，     5\nlogger_level: \"#info#\"\n\n# 输出重定向\nredirect: \"/dev/null\"\n\n# 随机数种子，null的话就用程序启动的时间作为种子\nrandom_seed: \"#seed#\"\n\n# 是否每个进程重复执行命令\nloop: false\n\n# 每个命令的进程数量基数\n# 最终分配的每个执行任务的进程数量为\n# process_num_base + assets.value\n# 也就是说 执行每个命令的进程数 等于 下面这个数 加 cmds里面每个键对应的值\nprocess_num_base: 0\n\n# 格式：\n# 执行的指令: 分配的进程数量\n# 如果分配的进程数 量为0则不会分配进程\ncmds:\n    # 付款账单尾号 这样写可以发送随机数\n    \"#engine# #output# -O #redirect# -q #host#pay.aspx?action=query_valid&transaction_id=#random#\": 0\n    # 这个文件比较大 40MB\n    \"#engine# #output# -O #redirect# -q #host#Binary/install_data.rar\": 0\n    # 接下来就是些比较小的文件\n    \"#engine# #output# -O #redirect# -q #host#Binary/trill_random_frequency.bin\": 0 # 500 KB\n    \"#engine# #output# -O #redirect# -q #host#Binary/zhaolei_unfiltered_vocal_cords_sound.bin\": 0 # 247KB\n"

#define rep std::cout << BOLDRED << "我在" << __FILE__ << "文件的第" << __LINE__ << "行" << RESET << std::endl;

#define send kill   // 发送信号用的函数
#define START 16    // 启动信号
#define STOP SIGINT // 停止信号

// 类型设置区
typedef char          int8;
typedef __int16_t     int16;
typedef __int32_t     int32;
typedef __int64_t     int64;

typedef unsigned char uint8;
typedef uint16_t      uint16;
typedef uint32_t      uint32;
typedef uint64_t      uint64;

// 软件没有安装异常类
class NotInstalledException : public std::runtime_error
{  
public:  
    explicit NotInstalledException(const std::string&& what_arg)  
        : std::runtime_error(what_arg) {}  
};

std::vector<std::string>* split(const std::string &s, const std::string& split);

enum class ValueType
{
    UNKNOWN,
    INT,
    FLOAT,
    STRING,
    BOOL
};

// 重载 << 运算符
std::ostream& operator<<(std::ostream& out, ValueType type);
// 使用正则表达式判断是否包含字母
bool containsLetter(const std::string& str);
// 是否为整数
bool isInt(const std::string& str);
// 尝试解析字符串的类型
ValueType getValueType(const std::string& str);

// 是否为对应类型，如果不是直接抛出异常
void checkInt(const std::string input);
void checkBool(const std::string& input);
void checkFloat(const std::string& input);

#endif