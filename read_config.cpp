#include "read_config.h"

// Yaml NodeType枚举类型转字符串
std::string type[] = {
    "Undefined", 
  	"Null", 
  	"Scalar", 
  	"Sequence", 
  	"Map"
};

void init_config()
{
    std::string default_config = INIT_CONFIG;
    // 写入字符串到文件
    std::ofstream outfile("config.yaml");
    if (outfile.is_open())
    {
        outfile << default_config;
        outfile.close();
        std::cout
            << BOLDGREEN << "初始化配置到"
            << BOLDYELLOW << "config.yaml"
            << BOLDGREEN << "成功。"
            << RESET << std::endl;
    }
    else
    {
        std::cerr
            << RED << "错误：无法打开文件进行写入"
            << RESET << std::endl;
        exit(-1);
    }
}

// 包含默认值的参数解析
template<typename T>
bool get_value(YAML::Node& config, std::string&& key, T& value_output, T&& default_value)
{
    try
    {
        value_output = config[key].as<T>();
    }
    catch(YAML::TypedBadConversion<T> &e)
    {
        std::cout << BOLDCYAN << "警告：你的配置文件当中缺少项目 `" << RESET
                  << YELLOW << key
                  << BOLDCYAN << "`"
                  << RESET << std::endl;

        std::cout << BOLDCYAN << "使用默认值：" 
                  << BOLDWHITE  << default_value
                  << BOLDCYAN << "。" 
                  << RESET << std::endl;

        //std::cout << e.what() << std::endl;
        value_output = T(default_value); // 复制一个默认值
        return false;
    }
    return true;
}

// 不包含默认值的参数解析
template<typename T>
bool get_value(YAML::Node& config, std::string&& key, T& value_output)
{
    try
    {
        value_output = config[key].as<T>();
    }
    catch(YAML::TypedBadConversion<T> &e)
    {
        std::cout << BOLDRED << "错误：你的配置文件当中缺少项目 `" << RESET
                  << YELLOW << key
                  << BOLDRED << "`" 
                  << RESET << std::endl;
        std::cout << e.what() << std::endl;
        exit(-1);
        //return false;
    }
    return true;
}

void printCenter(const char* str, int width = TITLE_WIDTH)
{
    int len = strlen(str);
    int left_space, right_space;
    if (len >= width) {
        printf("%s", str);
    }
    else
    {
        left_space = (width - len) / 2;
        right_space = width - len - left_space;
        printf("%*s%s%*s", left_space, "", str, right_space, "");
    }
}

std::string parse_command(int argc, char *argv[])
{
    std::string config_path;
    // 只有一个参数就是./dashicao
    // 这个时候显示打印信息
    if(argc == 1)
    {
        TITLE;
        INTRO;
        
        std::cout
            << std::endl
            << BOLDWHITE << "运行"
            << YELLOW << argv[0]
            << " "
            << "--help"
            << BOLDWHITE << "查看更多。" << RESET
            << std::endl;
        exit(-1);
    }

    // 两个参数可能是./dashicao -c
    // 或者./dashicao -cconfig.yaml
    // 或者是./dashicao -h
    // ./dashicao --help
    // 。/dashicao -i
    std::string second_arg;
    if(argc == 2)
    {
        second_arg = argv[1];
        // --help -help --h -h
        if(second_arg == "--h" || second_arg == "-h" || second_arg == "--help" || second_arg == "-help")
        {
            TITLE;
            INTRO;
            std::cout
                << BOLDMAGENTA << "\n钱虎CPP©2024版权所有" << std::endl
                << BOLDYELLOW << "\n命令行参数：\n"
                << BOLDWHITE;
            //打印帮助信息
            printf("%-20s: %s\n","-h, --help","打印这条帮助信息并退出");
            printf("%-20s: %s\n","-c, --config","指定运行时使用的YAML配置文件");
            printf("%-20s: %s\n","-i, --init","在当前目录下初始化配置文件到config.yaml");
            
            std::cout << RESET;
            exit(-1);
        }
        // --config -config --c -c
        else if(second_arg == "-c" || second_arg == "--c" || second_arg == "-config" || second_arg == "--config")
        {
            std::cerr
                << RED << "错误：选项" 
                << YELLOW << second_arg
                << RED << "不包含任何参数。"
                << RESET << std::endl;
            exit(-1);
        }
        // -i --i -init --init
        else if(second_arg == "-i" || second_arg == "--i" || second_arg == "-init" || second_arg == "--init")
        {
            init_config();
            exit(0);
        }
        // 选项和参数一起传递
        else if(second_arg.substr(0, 2) == "-c"
                 || second_arg.substr(0, 3) == "--c"
                 || second_arg.substr(0, 7) == "-config" 
                 || second_arg.substr(0, 8) == "--config")
        {
            if(second_arg.substr(0, 8) == "--config")
            {
                config_path = second_arg.substr(8);
            }
            else if(second_arg.substr(0, 7) == "-config")
            {
                config_path = second_arg.substr(7);
            }
            else if(second_arg.substr(0, 3) == "--c")
            {
                config_path = second_arg.substr(3);
            }
            else if(second_arg.substr(0, 2) == "-c")
            {
                config_path = second_arg.substr(2);
            }
        }
        else
        {
            std::cerr
                << RED << "错误：未识别的选项：" 
                << YELLOW << second_arg
                << RESET << std::endl;
            exit(-1);
        }
    }

    // 三个参数可能是./dashicao -c config.yaml
    // 或者是别的参数如./dashicao -m dsc.bin（错误）
    // ./dashicao -i aaa
    if(argc == 3)
    {
        second_arg = argv[1];
        if(second_arg == "-c" || second_arg == "--c" || second_arg == "-config" || second_arg == "--config")
        {
            config_path = argv[2];
        }
        // ./dashicao -i error
        else if(second_arg == "-i" || second_arg == "--i" || second_arg == "-init" || second_arg == "--init")
        {
            std::cerr
                << RED << "错误：未识别的参数：" 
                << YELLOW << argv[2]
                << RESET << std::endl;
            exit(-1);   
        }
        else
        {
            std::cerr
                << RED << "错误：未识别的选项：" 
                << YELLOW << second_arg
                << RESET << std::endl;
            exit(-1);   
        }
    }

    // 大于等于四个参数直接就是错误的
    if(argc >= 4)
    {
        std::cerr
                << RED << "错误：传递了四个及以上的参数，请检查你的命令行参数。" 
                << RESET << std::endl;
        exit(-1);
    }

    return config_path;
}

void typeCheck(const YAML::Node& input, const YAML::NodeType::value& except_type)
{
    if(except_type != input.Type()) // 如果类型错误
    {
        std::cerr << RED << "YAML配置文件数据类型错误，请检查一下 " << input << " 值的类型：" << RESET << std::endl;
        std::cerr 
                << "期望类型："
                << BOLDBLUE << type[except_type] << RESET
                << "，"
                << "实际类型："
                << BOLDRED << type[input.Type()] << RESET
                << std::endl;
        throw std::invalid_argument("YAML配置文件数据类型错误");
    } 
}

// 解析文件到一个类里面
// 虽然宏不展开，但是应当对对应的宏进行类型判断，确保宏在展开之后数据类型是正确的
// 也就是说我们需要临时展开一下宏
Config::Config(const std::string& file_path)
{
    mProcess_id = 0;
    mSeed = 0;
    mInit = false;
    // 类型别名 c++不允许非常量的临时引用作为参数，所以要加const
    const auto& MapType = YAML::NodeType::value::Map; 
    const auto& ScalarType = YAML::NodeType::value::Scalar;
    std::string temp; // 把字符串里面的宏展开到这个变量里面
    std::string current_key; // 当前正在加载的key

    try
    {
        // 通过路径加载文件
        auto config = YAML::LoadFile(file_path);
        
        // vars
        // 保留字
        mVars.insert({ "process_id", "0" });
        mVars.insert({ "random", "000000" });

        YAML::Node vars;
        current_key = "vars";
        get_value(config, std::string("vars"), vars);
        typeCheck(vars, MapType); // 类型检查
        // Map类型即为键值对，yaml-cpp中将Map类型的节点，使用pair类型进行遍历
        for (YAML::const_iterator it = vars.begin(); it != vars.end(); ++it) 
        {
            // 获取当前遍历的节点
            YAML::Node key_ = it->first;
            YAML::Node value_ = it->second;

            // 先检查是不是标量
            typeCheck(key_, ScalarType);
            typeCheck(value_, ScalarType);

            // 准备存储
            std::string key, value;
            
            key = key_.as<std::string>();
            value = value_.as<std::string>();
            if(key == "random" or key == "process_id")
            {
                std::cerr << "vars中不允许使用key：" << key << std::endl;
                std::cerr << "random和process_id为保留字" << std::endl;
                exit(-1);
            }
            mVars.insert({ key, value });
        }

        // logOutputConsole
        current_key = "logOutputConsole";
        get_value(config, std::string("logOutputConsole"), mLogOutputConsole);
        temp = this -> replace_all_variable(mLogOutputConsole); // 需要检查类型 bool
        checkBool(temp);

        // logger_dir
        get_value(config, std::string("logger_dir"), mLogger_dir);

        // logger_level
        current_key = "logger_level";
        get_value(config, std::string("logger_level"), mLogger_level);
        temp = this -> replace_all_variable(mLogger_level); // 需要检查类型 int
        checkInt(temp);

        // redirect
        get_value(config, std::string("redirect"), mRedirect);

        // random_seed
        current_key = "random_seed";
        get_value(config, std::string("random_seed"), mRandom_seed);
        temp = this -> replace_all_variable(mRandom_seed); // 需要检查类型 int
        checkInt(temp);

        // loop
        current_key = "loop";
        get_value(config, std::string("loop"), mLoop);
        temp = this -> replace_all_variable(mLoop); // 需要检查类型 bool
        checkBool(temp);

        // process_num_base
        current_key = "process_num_base";
        get_value(config, std::string("process_num_base"), mProcess_num_base);
        temp = this -> replace_all_variable(mProcess_num_base); // 需要检查类型 int
        checkInt(temp);

        // cmds
        // 需要将宏进行展开检查类型
        YAML::Node cmds;
        get_value(config, std::string("cmds"), cmds);
        typeCheck(cmds, MapType); // 类型检查
        for (YAML::const_iterator it = cmds.begin(); it != cmds.end(); ++it) 
        {
            // 获取当前遍历的节点
            YAML::Node key_ = it->first;
            YAML::Node value_ = it->second;

            // 先检查是不是标量
            typeCheck(key_, ScalarType);
            typeCheck(value_, ScalarType);

            // 准备存储
            std::string key, value;
            
            current_key = key = key_.as<std::string>();
            value = value_.as<std::string>();
            temp = this -> replace_all_variable(value);
            checkInt(temp);

            mCmds.push_back({ key, value });
        }
    }
    catch(YAML::BadFile &e)
    {
        std::cerr << "YAML配置文件读取出错！请检查你的配置文件。" << std::endl;
        std::cerr << "错误信息：" << std::endl;
        std::cerr << e.what() << std::endl;
        exit(-1);
    }
    catch(std::invalid_argument &e)
    {
        std::cerr << "YAML配置文件参数数据类型错误！" << std::endl;
        std::cerr << "检查 " << current_key << " 对应的值的数据类型时出现了错误。" << std::endl;
        std::cerr << "请检查你的配置文件。" << std::endl;
        std::cerr << "错误信息：" << std::endl;
        std::cerr << e.what() << std::endl;
        exit(-1);
    }
    mInit = true;
}

// 配置文件字符串宏展开相关
// 干掉字符串两遍的#
// 比如输入#random#输出random
// 用于查找字典的前处理步骤
inline std::string Config::unpack(const std::string& input)
{
    // 如果开头结尾有#
    if (input[0] == '#' and input[input.length() - 1] == '#')
    {
        return input.substr(1, input.length() - 2);
    }
    else
        return input;
}

// 将input字符串内指定变量替换为指定的内容
// 参数：要被操作的字符串 替换哪个变量 替换成什么东西
std::string Config::replace_variable(const std::string& input, const std::string& variable, const std::string& replacement)
{
    // 如果已经初始化并且使用了保留字
    if(mInit and (variable == "#random#" or variable == "#process_id#"))
    {
        std::string result = "";
        if(variable == "#random#")
        {
            // 六位随机数
            char n[7];
            sprintf(n, "%06d", get_random_number());
            result = std::string(n);
        }
        else if(variable == "#process_id#")
        {
            result = std::to_string(mProcess_id);
        }
        std::regex pattern(variable);  // 匹配单词
        return std::regex_replace(input, pattern, result);
    }
    else
    {
        // 如果没有初始化的话那就不去调用函数
        std::regex pattern(variable);  // 匹配单词
        return std::regex_replace(input, pattern, replacement);
    }
}

// 将所有的字符串内的#\b\w+?\b#替换为字典内对应的词
std::string Config::replace_all_variable(const std::string& text)
{
    auto output = std::string(text);
    std::vector<std::string> all_vars;
    std::regex pattern(R"(#\b\w+?\b#)");  // 匹配单词

    // 创建匹配的迭代器
    std::sregex_iterator words_begin = std::sregex_iterator(text.begin(), text.end(), pattern);
    std::sregex_iterator words_end = std::sregex_iterator();

    // 遍历所有匹配
    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::smatch match = *i;
        std::string match_str = match.str();
        all_vars.push_back(match_str);
    }

    // 循环替换所有变量
    for (std::string& var: all_vars)
    {
        // 先查变量是否已经被定义
        auto it = mVars.find(unpack(var));
        if (it != mVars.end()) // 如果找到了
        {
            output = replace_variable(output, var, it->second);
        }
        else // 如果没有被找到
        {
            output = replace_variable(output, var, unpack(var));
        }
    }
    return output;
}

void Config::init_random(unsigned int seed)
{
    mSeed = seed;
    mGen = std::mt19937(mSeed);
    mDis = std::uniform_int_distribution<>(0, 999999);
}