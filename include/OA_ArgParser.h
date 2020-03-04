#ifndef _OPENAPIXX_ARGPARSER_H_
#define _OPENAPIXX_ARGPARSER_H_

#include <string>
#include <vector>
#include <map>
#include <iostream>

namespace OpenAPIxx {

/**
 * 解析命令行参数
 * 支持短参数和长参数
 * 支持flag, value, value_append三种类型
 * 支持flag类型的短参数的简写，例如-a -b -c可以写成-abc
 * 支持多個參數用於同一功能
 * 可以自动生成帮助信息

 * 非APPEND类型多次设置会覆盖之前的
 * 同一功能的多个参数，设置一个参数后，访问其余参数都可以得到参数值
 */

enum ArgType {
    ARGTYPE_FLAG        = 1,
    ARGTYPE_VALUE       = 2,
    ARGTYPE_VALUEAPPEND = 3,
    ARGTYPE_UNKNOWN     = 4
};

class ArgValue {
public:
    ArgValue();
    ArgValue(bool b);
    ArgValue(const char* str);
    ArgValue(const std::string& str);
    friend std::ostream& operator<<(std::ostream& out, const ArgValue& argValue);
    void set(bool b);
    void set(const char* str);
    void set(const std::string& str);
    void reset();

    bool isSet;
    bool boolVal;
    std::string strVal;
    ArgType type;
};

struct ArgInfo;
typedef std::map < std::string, OpenAPIxx::ArgInfo*> ArgsInfo;
typedef std::map < std::string, OpenAPIxx::ArgValue*> ArgsValue;

class ArgParser {
public:
    ArgParser();
    explicit ArgParser(const std::string& programName);
    ~ArgParser();

    /**
     * 设置参数的信息
     * name:            参数名称，长短形式都可以
     * type:            参数类型
     * omitable:        参数是否可省略
     * defaultValue:    参数默认值，可填bool或者字符串类型
     * discription:     参数描述信息，显示在帮助信息中
     * meta:            非ARGTYPE_FLAG类型参数的meta字段，显示在帮助信息中
     */
    int setArg(const std::string& name, ArgType type, bool omitable,
        const ArgValue& defaultValue, const std::string& discription, const std::string& meta);
    int setMultiArg(const std::vector<std::string>& names, ArgType type, bool omitable,
        const ArgValue& defaultValue, const std::string& discription, const std::string& meta);

    /**
     * 解析参数
     * 解析成功返回0，否则表示失败，失败信息保存在errorMessage中
     * 解析后的参数保存在args中
     */
    int parseArg(int argc, const char** argv, ArgsValue& args, std::string& errorMessage);

    /**
     * 生成幫助信息
     * 成功返回0，否則表示失敗。
     */
    std::string generateHelpMessage();

private:
    /**
     * 判断argv中的一个字符串是否是合法的参数
     * 合法返回true，否则返回false
     */
    bool _isValidArgFormat(const std::string& arg);

    /**
     * 把簡寫的短參數拆開
     * 其他類型的參數不拆
     * 返回一個vector，包含處理後的參數
     */
    std::vector<std::string> _splitArg(const char* arg);

    ArgsInfo m_argsInfo;                // 所有參數名與對應的參數值
    std::vector<ArgInfo*> m_argInfos;   // 所有參數值的指針的數值
    std::string m_programName;
};

}

typedef OpenAPIxx::ArgParser OAArgParser;

#endif//_OPENAPIXX_ARGPARSER_H_
