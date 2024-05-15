// 工具库
#pragma once
#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/time.h>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <atomic>
#include <fstream>
#include <json/json.h>

#include "../comm/cppjieba/Jieba.hpp"

using namespace std;

const string path = "./temp/";

// cppjieba 词库路径
const string DICT_PATH = "../comm/cppjieba/dict/jieba.dict.utf8";
const string HMM_PATH = "../comm/cppjieba/dict/hmm_model.utf8";
const string USER_DICT_PATH = "../comm/cppjieba/dict/user.dict.utf8";
const string IDF_PATH = "../comm/cppjieba/dict/idf.utf8";
const string STOP_WORD_PATH = "../comm/cppjieba/dict/stop_words.utf8";

// 编程语言
enum progra_lage
{
    CPP,
    C
};

class TimeUtil
{
public:
    // 获取时间戳
    static string Gettimest()
    {
        time_t time = chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        stringstream ss;
        ss << put_time(std::localtime(&time), "%Y年%m月%d日 %H点%M分%S秒");
        return ss.str();
    }

private:
};

class PathUtil
{
public:
    // 添加后缀+前缀
    static string Addsuffix(const string &file_name, const string &suffix)
    {
        string res = path;
        res += file_name;
        res += suffix;

        return res;
    }

    // 构建完整源文件路径名+后缀
    static string Src(const string &file_name, const progra_lage &lag)
    {
        if (lag == CPP)
            return Addsuffix(file_name, ".cpp");
        else
            return "";
    }

    // 构建完整程序路径名+后缀
    static string Erc(const string &file_name)
    {
        return Addsuffix(file_name, ".exe");
    }

    // 构建完整标准编译错误路径名+后缀
    static string Comilperror(const string &file_name)
    {
        return Addsuffix(file_name, ".comilp_error");
    }

    // 构建完整的运行程序之后的路径//
    static string Stdin(const string &file_name)
    {
        return Addsuffix(file_name, ".stdin");
    }

    static string Stdout(const string &file_name)
    {
        return Addsuffix(file_name, ".stdout");
    }

    static string Stdrun_err(const string &file_name)
    {
        return Addsuffix(file_name, ".stderr");
    }

private:
};

class FileUtil
{
public:
    // 获取文件是否存在
    static bool Is_file_exists(const string &path_name)
    {
        struct stat st;
        if (stat(path_name.c_str(), &st) == 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    // 形成唯一文件名
    static string Uniqe_file_name()
    {
        static atomic_uint id(0);
        id++;
        string ms = TimeUtil::Gettimest();
        string uid = to_string(id);

        return uid + "_" + ms;
    }

    // 向文件写入字符串
    static bool WrtieFile(const string &src_path, const string &s)
    {
        ofstream out(src_path);

        if (out.is_open() == false)
            return false;

        out.write(s.c_str(), s.size());
        out.close();
        return true;
    }

    // 读取文件转字符串
    static bool ReadFile(const string &src_path, string &s, bool keep = false)
    {
        s.clear();

        ifstream in(src_path);
        if (in.is_open() == false)
            return false;

        string line;
        while (getline(in, line))
        {
            s += line;
            s += (keep == true ? "\n" : "");
        }
        in.close();
        return true;
    }

private:
};

// 字符串工具
class StringUtil
{
public:
    // json转utf-8-string
    static string JsonToString(const Json::Value &root)
    {
        static Json::Value def = []()
        {
            Json::Value def;
            Json::StreamWriterBuilder::setDefaults(&def);
            def["emitUTF8"] = true;
            return def;
        }();

        ostringstream stream;
        Json::StreamWriterBuilder stream_builder;
        stream_builder.settings_ = def; // Config emitUTF8
        unique_ptr<Json::StreamWriter> writer(stream_builder.newStreamWriter());
        writer->write(root, &stream);
        return stream.str();
    }

    // 字符串切分
    static void String_split(const string &src, const string &c, vector<string> &target)
    {
        target.clear(); // 清空目标向量

        size_t start = 0, end = 0;
        while ((end = src.find(c, start)) != string::npos)
        {
            // 将从 start 到 end 之间的子串加入目标向量
            target.push_back(src.substr(start, end - start));
            start = end + c.length(); // 更新起始位置
        }

        // 处理最后一个分隔符之后的子串
        target.push_back(src.substr(start));
    }

private:
};

// 自定义比较工具
class CmpUtil
{
public:
    // 难度比较
    static bool Starcmp(const string &a, const string &b)
    {
        int n1, n2;
        if (a == "简单")
            n1 = 0;
        else if (a == "中等")
            n1 = 1;
        else
            n1 = 2;

        if (b == "简单")
            n2 = 0;
        else if (b == "中等")
            n2 = 1;
        else
            n2 = 2;

        return n1 < n2;
    }

private:
};

// 编程语言工具
class Prog_language_Util
{
public:
    static bool lang_string_to(const string &lag_name, progra_lage &out_lag)
    {
        if (lag_name == "CPP" || lag_name == "c++" || lag_name == "cpp" || lag_name == "C++")
        {
            out_lag = CPP;
            return true;
        }
        else
        {
            return false;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////////////
// cppjieba 字符串分词 工具
class Jieba_util
{
public:
    Jieba_util()
    {
        ifstream in(STOP_WORD_PATH);
        if (!in.is_open())
        {
            cout << "open SOTP_WORD error" << endl;
        }

        string line;
        while (getline(in, line))
        {
            stop_words[line]++;
        }

        in.close();
    }

    // 分词
    static void Cutstring(const string &src, vector<string> &vc, bool remove_stop_word = true)
    {
        if (stop_words.size() == 0)
        {
            Jieba_util();
        }

        vector<string> res;
        jieba.Cut(src, vc);

        // 是否去掉暂停词
        if (remove_stop_word)
        {
            Remostop_wrod(vc, res);
            vc = move(res);
        }
    }

private:
    // 去除暂停词
    static void Remostop_wrod(const vector<string> &src, vector<string> &res)
    {
        for (int i = 0; i < src.size(); i++)
        {
            if (stop_words[src[i]] == 0)
            {
                res.push_back(src[i]);
            }
        }
    }

private:
    static cppjieba::Jieba jieba;
    static unordered_map<string, int> stop_words;
};
cppjieba::Jieba Jieba_util::jieba(DICT_PATH, HMM_PATH, USER_DICT_PATH, IDF_PATH, STOP_WORD_PATH);
unordered_map<string, int> Jieba_util::stop_words;
//////////////////////////////////////////////////////////////////////////////////////////////////
