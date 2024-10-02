#include "utils.h"

std::vector<std::string>* split(const std::string &str, const std::string& split)
{
    auto tokens = new std::vector<std::string>;
	std::regex reg(split);		// 匹配split
	std::sregex_token_iterator pos(str.begin(), str.end(), reg, -1);
	decltype(pos) end;              // 自动推导类型 
	for (; pos != end; ++pos)
	{
		tokens->push_back(pos->str());
	}
    return tokens;
}

// 重载 << 运算符
std::ostream& operator<<(std::ostream& out, ValueType type)
{
    switch (type) {
    case ValueType::UNKNOWN:
        out << "UNKNOWN";
        break;
    case ValueType::INT:
        out << "INT";
        break;
    case ValueType::FLOAT:
        out << "FLOAT";
        break;
    case ValueType::STRING:
        out << "STRING";
        break;
    case ValueType::BOOL:
        out << "BOOL";
        break;
    default:
        out << "Unknown ValueType";
        break;
    }
    return out;
}

// 使用正则表达式判断是否包含字母
bool containsLetter(const std::string& str)
{
    std::regex letter_regex("[a-zA-Z]");
    return std::regex_search(str, letter_regex);
}

// 是否为整数
bool isInt(const std::string& str)
{
    std::regex numeric_regex("^[0-9]+$");
    return std::regex_match(str, numeric_regex);
}

// 尝试解析字符串的类型
ValueType getValueType(const std::string& str)
{
    if (str == "true" || str == "false")
    {
        return ValueType::BOOL;
    }
    if (containsLetter(str) || "." == str) // 如果包含字母或者只有一个小数点
    {
        return ValueType::STRING; // 那一定是字符串类型
    }
    if (isInt(str)) // 如果是整数
    {
        return ValueType::INT; // 是整数类型
    }
    if (str.find('.') != std::string::npos && (std::count(str.begin(), str.end(), '.') == 1))
    {
        return ValueType::FLOAT; // 小数类型
    }

    // 如果所有逻辑都没有命中，默认为字符串类型
    return ValueType::STRING;
}

// 类型检查，不合格就抛出异常那种
void checkInt(const std::string input)
{
    if(getValueType(input) != ValueType::INT)
    {
        throw std::invalid_argument("字符串" + input + " int类型检查失败！");
    }
}

void checkBool(const std::string& input)
{
    if(getValueType(input) != ValueType::BOOL)
    {
        throw std::invalid_argument("字符串" + input + " bool类型检查失败！");
    }
}

void checkFloat(const std::string& input)
{
    if(getValueType(input) != ValueType::FLOAT)
    {
        throw std::invalid_argument("字符串" + input + " float类型检查失败！");
    }
}