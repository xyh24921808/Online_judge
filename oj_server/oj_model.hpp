#pragma once
// 题库操作模块
// my_sql版本 MYSQL_MODEL

#ifdef MYSQL_MODLE
#include <mysql/mysql.h>
#endif

#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <fstream>
#include <thread>
#include <unordered_map>

#include "../comm/util.hpp"
#include "../comm/log.hpp"

#ifdef MYSQL_MODLE
const string oj_questions = "oj_questions";
const string host = "127.0.0.1";
const string user = "oj_FX";
const string password = "xyh123456";
const string db = "oj";
const int sql_port = 3306;
#endif

#ifndef MYSQL_MODLE
const string questions_list = "./questions/questions.list";
const string questions_path = "./questions/";
#endif

struct Prog_lang_info
{
    string header; // 题目语言对应预设代码
    string tail;   // 题目语言对应测试用例
    string main;   // 题目语言对应测试运行
};

struct Questions
{
public:
    string number; // 题目编号
    string title;  // 题目标题
    string star;   // 题目难度

    int cpu_limit; // 时间要求
    int mem_limit; // 空间要求

    string desc; // 题目描述
public:
    // 增加语言的预设代码 测试用例 测试主体
    void Add_progar_all(const progra_lage &lag, const string &header, const string &tail, const string &test_run)
    {
        if (prog_info.find(lag) == prog_info.end())
            prog_info[lag] = {header, tail, test_run};
    }

    // 获取语言的预设代码 测试用例 测试主体
    bool Get_progar_all(const progra_lage &lag, string &out_headr, string &out_tail, string &out_test)
    {
        if (prog_info.find(lag) == prog_info.end())
            return false;

        out_headr = prog_info[lag].header;
        out_tail = prog_info[lag].tail;
        out_test = prog_info[lag].main;
        return true;
    }

    // 修改语言的预设代码 测试用例 测试主体
    bool Modify_progar_all(const progra_lage &lag, const string &header, const string &tail, const string &test_run)
    {
        if (prog_info.find(lag) == prog_info.end())
            return false;

        prog_info[lag] = {header, tail, test_run};
        return true;
    }

    // 获取语言预设代码
    bool Get_progar_header(const progra_lage &lag, string &out_string)
    {
        if (prog_info.find(lag) == prog_info.end())
        {
            return false;
        }
        out_string = prog_info[lag].header;
        return true;
    }

    // 获取语言测试用例代码
    bool Get_progar_tail(const progra_lage &lag, string &out_string)
    {
        if (prog_info.find(lag) == prog_info.end())
        {
            return false;
        }
        out_string = prog_info[lag].tail;
        return true;
    }

    // 获取语言测试运行代码
    bool Get_progar_test_main(const progra_lage &lag, string &out_string)
    {
        if (prog_info.find(lag) == prog_info.end())
        {
            return false;
        }
        out_string = prog_info[lag].main;
        return true;
    }

private:
    unordered_map<progra_lage, Prog_lang_info> prog_info; // 编程语言的预设代码 测试用例 测试主体
};

class Model
{
public:
    Model()
    {
#ifdef MYSQL_MODLE
        assert(Load_quest_list());
#endif

#ifndef MYSQL_MODLE
        assert(Load_quest_list(questions_list));
#endif

        Init_index();
    }

#ifndef MYSQL_MODLE
    ~Model()
    {
        ///////////////////////////////
    }
#endif

// 数据库版
#ifdef MYSQL_MODLE
    // 加载题目到内存中
    bool Load_quest_list()
    {
        MYSQL *my_ptr = mysql_init(nullptr);

        if (mysql_real_connect(my_ptr, host.c_str(), user.c_str(), password.c_str(), db.c_str(), sql_port, nullptr, 0) == nullptr)
        {
            LOG(FATA) << "致命错误 连接数据库失败" << endl;
            return false;
        }

        LOG(INFO) << "连接数据库成功" << endl;

        string select_sql = "SELECT * FROM " + oj_questions;
        if (mysql_query(my_ptr, select_sql.c_str()) != 0)
        {
            LOG(WARING) << "mysql query fate" << endl;
            return false;
        }

        MYSQL_RES *res = mysql_store_result(my_ptr);

        int row = mysql_num_rows(res);
        int col = mysql_num_fields(res);

        for (int i = 0; i < row; i++)
        {
            MYSQL_ROW line = mysql_fetch_row(res);
            Questions q;

            q.number = line[0];
            q.title = line[1];
            q.star = line[2];
            q.desc = line[3];

            q.header = line[4];
            q.tail = line[5];
            q.tail_main = line[6];
            q.cpu_limit = atoi(line[7]);
            q.mem_limit = atoi(line[8]);

            _questions[q.number] = q;
        }

        mysql_close(my_ptr);
        return true;
    }
#endif

// 文件版
#ifndef MYSQL_MODLE
    // 加载题目到内存中
    bool Load_quest_list(const string &questison_list)
    {
        ifstream in(questison_list);

        if (in.is_open() == false)
        {
            LOG(FATA) << "致命错误 无法加载题库列表" << endl;
            return false;
        }

        string line;
        // 按行读取list 再进行切分
        while (getline(in, line))
        {

            vector<string> tokens;
            StringUtil::String_split(line, " ", tokens);

            if (tokens.size() != 5)
            {
                LOG(WARING) << "加载单个题目失败" << endl;
                continue;
            }

            Questions q;
            q.number = tokens[0];
            q.title = tokens[1];
            q.star = tokens[2];
            q.cpu_limit = stoi(tokens[3]);
            q.mem_limit = stoi(tokens[4]);

            string path = questions_path;
            path += q.number;

            FileUtil::ReadFile(path + "/desc.txt", q.desc, true);

            Load_programming_language_text(q, path, CPP, "cpp");

            _questions.insert({q.number, q});
        }
        in.close();
        LOG(INFO) << "题库加载完毕" << endl;
        return true;
    }
#endif

    // 初始化搜索索引
    void Init_index()
    {
        for (auto &[number, quest] : _questions)
        {
            string str_number = number;
            string desc = quest.desc;
            string title = quest.title;

            vector<string> src_desc;
            vector<string> src_title;

            Jieba_util::Cutstring(desc, src_desc, false);
            Jieba_util::Cutstring(title, src_title, false);

            for (auto &word : src_title)
            {
                _questions_back_index[word].push_back(str_number);
            }

            for (auto &word : src_desc)
            {
                _questions_back_index[word].push_back(str_number);
            }

            _questions_back_index[number].push_back(str_number);
        }
        LOG(INFO) << "题库倒排索引初始化完毕" << endl;
    }

    // 获取所有题目信息
    bool Get_all_quest(vector<Questions> &out)
    {
        if (_questions.size() == 0)
        {
            LOG(ERROR) << "获取题库失败" << endl;
            return false;
        }
        for (auto [key, val] : _questions)
        {
            out.push_back(val);
        }
        return true;
    }

    // 获取搜索关键词题库信息
    bool Get_search_questlist(const string &word, vector<Questions> &out)
    {
        // 对搜索词进行分词
        vector<string> vc_word;
        Jieba_util::Cutstring(word, vc_word,false);

        out.clear();

        // 统计单词在题目中出现的次数
        unordered_map<string, int> quest_count;
        for (auto &str : vc_word)
        {
            if (_questions_back_index.find(str) != _questions_back_index.end())
            {
                for (auto &number : _questions_back_index[str])
                {
                    quest_count[number]++;
                }
            }
        }

        vector<pair<string, int>> vc_quest_count;
        for (auto &[number, count] : quest_count)
        {
            vc_quest_count.push_back({number, count});
        }

        // 排序题库
        sort(vc_quest_count.begin(), vc_quest_count.end(),
        [](pair<string, int> &a, pair<string, int> &b) -> bool
        {
            if (a.second == b.second)
                return a.first < b.first;
            else
                return a.second > b.second;
        });

        for (auto &[number, count] : vc_quest_count)
        {
            out.push_back(_questions[number]);
        }

        return true;
    }

    // 获取指定题目信息
    bool Get_one_quest(const string &quest_number, Questions &out)
    {
        if (_questions.find(quest_number) == _questions.end())
        {
            LOG(ERROR) << "获取题目失败" << endl;
            return false;
        }

        out = _questions[quest_number];
        return true;
    }

    // 增加题目
    bool Add_quest(const Questions &in)
    {
        unique_lock<mutex> lock(_mtx);
        string number = in.number;
        if (_questions.find(number) != _questions.end())
        {
            LOG(INFO) << "增加题目失败 已存在相同编号题目" << endl;
            return false;
        }

        _questions[number] = in;
        return true;
    }

    // 修改题目描述信息 标题 描述 难度
    bool Modify_quest_info(const string &number, const string &title, const string &desc, const string &star)
    {
        unique_lock<mutex> lock(_mtx);
        if (_questions.find(number) == _questions.end())
        {
            LOG(ERROR) << "修改题目描述失败 不存在该题目" << endl;
            return false;
        }

        _questions[number].title = title;
        _questions[number].desc = desc;
        _questions[number].star = star;
        return true;
    }

    // 修改题目代码 编程语言 预设代码 测试用例代码 测试运行主体
    bool Modify_quest_code(const string &number, const progra_lage &type, const string &header, const string &tail, const string &test_run)
    {
        unique_lock<mutex> lock(_mtx);
        if (_questions.find(number) == _questions.end())
        {
            LOG(ERROR) << "修改题目代码失败 不存在该题目" << endl;
            return false;
        }

        return _questions[number].Modify_progar_all(type, header, tail, test_run);
    }

    // 修改题目
    bool Modify_quest(const string &number, const Questions &in)
    {
        unique_lock<mutex> lock(_mtx);
        if (_questions.find(number) == _questions.end())
        {
            LOG(ERROR) << "修改题目失败 不存在该题目" << endl;
            return false;
        }

        _questions[number] = in;
        return true;
    }

    // 删除指定题目
    bool Del_quest(const string &number)
    {
        if (_questions.find(number) == _questions.end())
        {
            LOG(ERROR) << "删除题目失败 不存在该题目" << endl;
            return false;
        }

        _questions.erase(number);
        return true;
    }

private:
#ifndef MYSQL_MODLE
    // 加载编程语言对应的文本
    void Load_programming_language_text(Questions &q, const string &path, progra_lage language_name, const string &file_nameback)
    {
        string header, tail, main;
        FileUtil::ReadFile(path + "/header." + file_nameback, header, true);
        FileUtil::ReadFile(path + "/tail." + file_nameback, tail, true);
        FileUtil::ReadFile(path + "/tail_main." + file_nameback, main, true);
        q.Add_progar_all(language_name, header, tail, main);
    }
#endif

private:
    // 用哈希表题号来映射题目详细信息
    unordered_map<string, Questions> _questions;

    // 题库标题内容的倒排索引
    unordered_map<string, vector<string>> _questions_back_index;

    // 线程锁
    mutex _mtx;
};