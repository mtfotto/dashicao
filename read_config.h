#ifndef DASHICAO_READ_CONFIG_H
#define DASHICAO_READ_CONFIG_H

#include "utils.h"

// 打印软件简介
#define INTRO std::cout\
                    << BOLDWHITE << "大市操是一款由钱虎CPP开发的\n"\
                                    "高性能并发式多进程任务执行框架。\n"\
                                    "软件支持通过使用YAML配置文件对参数进行配置。"\
                                 << RESET << std::endl

// 打印编译时间
#define BUILD_TIME std::cout \
                    << BOLDWHITE << "构建时间："\
                    << BOLDCYAN << __DATE__ << " " << __TIME__\
                    << RESET << std::endl

// 标题宽度
#define TITLE_WIDTH 50

// 显示标题
#define TITLE \
        std::cout\
            << BOLDYELLOW;\
        printCenter("大市操");\
        std::cout << std::endl;\
        BUILD_TIME

class Config
{
public:
    bool                                               mInit; // 有没有初始化完毕
    std::string                                        mLogOutputConsole;
    std::string                                        mLogger_dir;
    std::string                                        mLogger_level;
    std::string                                        mRedirect;
    std::string                                        mRandom_seed;
    std::string                                        mLoop;
    std::string                                        mProcess_num_base;
    int                                                mProcess_id; // 子进程自己的编号，用于替换#process_id#保留字
    std::map<std::string, std::string>                 mVars;
    std::vector<std::tuple<std::string, std::string> > mCmds;

    // 随机数相关
    unsigned int                                       mSeed; // 随机种子
    std::mt19937                                       mGen;  // Mersenne Twister生成器
    std::uniform_int_distribution<>                    mDis;  // 随机数的范围

    // 构造函数
    Config(const std::string& file_path);

    // 字符串宏展开相关
    inline std::string unpack(const std::string& input);
    std::string replace_variable(const std::string& input, const std::string& variable, const std::string& replacement);
    std::string replace_all_variable(const std::string& text);

    // 发生随机数
    void init_random(unsigned int seed);
    int get_random_number() { return mDis(mGen); }
};

void init_config();

template<typename T>
bool get_value(YAML::Node& config, std::string&& key, T& value_output, T&& default_value);

template<typename T>
bool get_value(YAML::Node& config, std::string&& key, T& value_output);

void printCenter(const char* str, int width);

std::string parse_command(int argc, char *argv[]);

void typeCheck(const YAML::Node& input, const YAML::NodeType::value& except_type);

#endif