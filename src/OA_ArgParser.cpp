#include "OA_ArgParser.h"
#include "OA_ErrorNumber.h"
#include "OA_ASCII.h"
#include <algorithm>
#include <sstream>

namespace OpenAPIxx {
    struct ArgInfo {
        ArgInfo() : omitable(true), type(ARGTYPE_FLAG) {}
        std::string genUsage() {
            std::string ret, name = names[0];
            switch (type) {
            case ARGTYPE_FLAG:          ret = name; break;
            case ARGTYPE_VALUE:         ret = name + " " + meta; break;
            case ARGTYPE_VALUEAPPEND:   ret = name + " " + meta + " ..."; break;
            default:                    break;
            }
            if (omitable == true) {
                ret = "[" + ret + "]";
            }
            return ret;
        }

        bool omitable;
        ArgType type;
        std::vector<std::string> names;
        std::string discription;
        std::string meta;
        ArgValue value;
        ArgValue defaultValue;
    };

    std::ostream & operator<<(std::ostream & out, const ArgValue & argValue) {
        switch (argValue.type) {
        case ARGTYPE_FLAG: 
            out << (argValue.boolVal == true ? "true" : "false");
            break;
        case ARGTYPE_VALUE:
        case ARGTYPE_VALUEAPPEND:
            out << argValue.strVal;
            break;
        default:
            out << "UNKNOWN";
        }
        return out;
    }
}

OpenAPIxx::ArgValue::ArgValue() : isSet(false), boolVal(false), type(ARGTYPE_UNKNOWN) {}

OpenAPIxx::ArgValue::ArgValue(bool b) : isSet(true), boolVal(b), type(ARGTYPE_FLAG) {}

OpenAPIxx::ArgValue::ArgValue(const char* str) : isSet(true), boolVal(false), strVal(str), type(ARGTYPE_VALUE) {}

OpenAPIxx::ArgValue::ArgValue(const std::string& str) : isSet(true), boolVal(false), strVal(str), type(ARGTYPE_VALUE) {
    // setArg/setMultiArg中的參數默認值
    // 在調用前無法確認類型
    // 在函數調用時修正
}

void OpenAPIxx::ArgValue::set(bool b) {
    boolVal = b; isSet = true;
}

void OpenAPIxx::ArgValue::set(const char* str) {
    strVal = str; isSet = true;
}

void OpenAPIxx::ArgValue::set(const std::string& str) {
    strVal = str; isSet = true;
}

void OpenAPIxx::ArgValue::reset() { boolVal = false; strVal = ""; isSet = false; }

OpenAPIxx::ArgParser::ArgParser() : m_programName("ProgramName") {}

OpenAPIxx::ArgParser::ArgParser(const std::string& programName) : m_programName(programName) {}

OpenAPIxx::ArgParser::~ArgParser() {
    for (size_t i = 0; i < m_argInfos.size(); i++) {
        delete m_argInfos[i];
    }
}

int OpenAPIxx::ArgParser::setArg(const std::string& name, ArgType type, bool omitable,
    const ArgValue& defaultValue, const std::string& discription, const std::string& meta) {
    return setMultiArg({ name }, type, omitable, defaultValue, discription, meta);
}

int OpenAPIxx::ArgParser::setMultiArg(const std::vector<std::string>& names, ArgType type, bool omitable,
    const ArgValue& defaultValue, const std::string& discription, const std::string& meta) {
    ArgInfo* info = new ArgInfo();
    for (auto it = names.begin(); it != names.end(); it++) {
        if (this->_isValidArgFormat(*it) == false) {
            return OA_ERR_ILLEGAL_ARG;
        }
        m_argsInfo[*it] = info;
    }
    m_argInfos.push_back(info);

    // 保存參數信息
    info->names = names;
    info->type = type;
    info->defaultValue = defaultValue;
    info->defaultValue.type = type;
    info->omitable = omitable;
    info->discription = discription;
    info->meta = meta;
    info->value.type = type;

    return OA_ERR_NO_ERROR;
}

#define OAARGPARSE_STATUS_ARG 0
#define OAARGPARSE_STATUS_VAL -1
int OpenAPIxx::ArgParser::parseArg(int argc, const char** argv, ArgsValue& args, std::string& errorMessage) {
    args.clear();
    errorMessage = "";

    // 構建參數名和參數值的映射
    for (auto it = m_argsInfo.begin(); it != m_argsInfo.end(); it++) {
        it->second->value.reset();
        for (size_t i = 0; i < it->second->names.size(); i++) {
            args[it->second->names[i]] = &(it->second->value);
        }
    }

    // 遍歷傳入的參數
    // 使用狀態機控制當前解析參數還是參數的值
    int status = OAARGPARSE_STATUS_ARG;
    std::string curArg;
    bool isShortArgs;
    std::map<std::string, ArgInfo*>::iterator infoIt;
    for (int i = 1; i < argc; i++) {
        if(status == OAARGPARSE_STATUS_ARG) {
            if(_isValidArgFormat(argv[i]) == false) {
                errorMessage = std::string("invalid arg format: ") + argv[i];
                return OA_ERR_OPERATION_FAILED;
            }
            // 獲取參數名
            curArg = argv[i];
            isShortArgs = false;
            auto splitedArg = _splitArg(argv[i]);
            if (splitedArg.size() > 1) {
                isShortArgs = true;
            }
            for(auto it = splitedArg.begin(); it != splitedArg.end(); it++) {
                // 參數是否存在
                infoIt = m_argsInfo.find(*it);
                if(infoIt == m_argsInfo.end()) {
                    errorMessage = std::string("unknow arg: ") + (*it); 
                    return OA_ERR_OPERATION_FAILED;
                }
                // 檢查簡寫短參數的類型
                switch (infoIt->second->type) {
                case ARGTYPE_FLAG:
                    args[*it]->set(true);
                    break;
                case ARGTYPE_VALUE:
                case ARGTYPE_VALUEAPPEND:
                    if (isShortArgs == true) {
                        errorMessage = std::string("format error: ") + argv[i];
                        return OA_ERR_OPERATION_FAILED;
                    }
                    status = OAARGPARSE_STATUS_VAL;
                    break;
                default:
                    errorMessage = std::string("internel error, unknown type:") + std::to_string(infoIt->second->type);
                    return OA_ERR_OPERATION_FAILED;
                }
            }
        } else { // status == OAARGPARSE_STATUS_VAL{
            // 保存value
            if (infoIt->second->type == ARGTYPE_VALUEAPPEND
                && args.find(curArg) != args.end()) {
                args[curArg]->set(args[curArg]->strVal + " " + argv[i]);
            } else {
                args[curArg]->set(argv[i]);
            }
            status = OAARGPARSE_STATUS_ARG;
        }
    } // foreach argv
    // 缺少value的情況
    if (status == OAARGPARSE_STATUS_VAL) {
        errorMessage = std::string("value is missing! arg:") + curArg;
        return OA_ERR_OPERATION_FAILED;
    }
    // 給未出現的參數設置默認值
    for (auto it = m_argsInfo.begin(); it != m_argsInfo.end(); it++) {
        std::string name = it->second->names[0];
        if (args[name]->isSet == false) {
            if (it->second->omitable == false) {
                errorMessage = "missing arg: " + name;
                return OA_ERR_OPERATION_FAILED;
            }
            *(args[name]) = it->second->defaultValue;
        }
    }
    args = args;
    return OA_ERR_NO_ERROR;
}

std::string OpenAPIxx::ArgParser::generateHelpMessage() {
    // 必選參數
    // 可選FLAG類型短參數
    // 其他參數

    std::stringstream ss;
    std::vector<std::string> nonOmitableArgs, omitableShortArgs, otherArgs;
    std::string name;
    ArgInfo* info;
    for (size_t i = 0; i < m_argInfos.size(); i++) {
        info = m_argInfos[i];
        name = info->names[0];
        if (info->omitable == false) {
            nonOmitableArgs.push_back(name);
        }
        else if (name.length() == 2 && info->type == ARGTYPE_FLAG) {
            omitableShortArgs.push_back(name);
        }
        else {
            otherArgs.push_back(name);
        }
    }
    std::sort(nonOmitableArgs.begin(), nonOmitableArgs.end());
    std::sort(omitableShortArgs.begin(), omitableShortArgs.end());
    std::sort(otherArgs.begin(), otherArgs.end());

    ss << "Usage: "<< std::endl <<"    " << m_programName << ' ';
    for (size_t i = 0; i < nonOmitableArgs.size(); i++) {
        ss << m_argsInfo[nonOmitableArgs[i]]->genUsage() << ' ';
    }
    std::string allShortOmitableFlagArg;
    for (size_t i = 0; i < omitableShortArgs.size(); i++) {
        allShortOmitableFlagArg.push_back(omitableShortArgs[i][1]);
    }
    if (allShortOmitableFlagArg.length() > 0) {
        ss << "[-" << allShortOmitableFlagArg << "] ";
    }
    for (size_t i = 0; i < otherArgs.size(); i++) {
        ss << m_argsInfo[otherArgs[i]]->genUsage() << ' ';
    }
    
    ss << std::endl << "Options:" << std::endl;
    std::string args;
    for (size_t i = 0; i < m_argInfos.size(); i++) {
        args.clear();
        for (size_t j = 0; j < m_argInfos[i]->names.size(); j++) {
            args += m_argInfos[i]->names[j] + ", ";
        }
        args.pop_back();
        args.pop_back();
        ss << args << std::endl << "    " << m_argInfos[i]->discription << std::endl;
    }
    return ss.str();
}

bool OpenAPIxx::ArgParser::_isValidArgFormat(const std::string& arg) {
    if (arg.length() < 2
    || arg == "--"
    || arg[0] != '-') {
        return false;
    }

    // 檢查“-”後面的內容的合法性，取值範圍a-z,A-Z,0-9
    size_t i = 1;
    if (arg[1] == '-') {
        i = 2;
    }
    for (; i < arg.length(); i++) {
        if (OAASCII::isAlphbateOrNumber(arg[i]) == false) {
            return false;
        }
    }
    return true;
}

// 用於獲取多個連續的flag類型短參數
std::vector<std::string> OpenAPIxx::ArgParser::_splitArg(const char* arg) {
    std::vector<std::string> ret;
    std::string arg_str(arg);

    // 多個flag短參數
    if (arg_str.length() > 2 && arg_str[1] != '-') {
        for (size_t i = 1; i < arg_str.length(); i++) {
            ret.push_back(std::string("-") + arg_str[i]);
        }
    }
    // 一個參數
    else {
        ret.push_back(arg_str);
    }

    return ret;
}

