#pragma once
// 题库操作模块
// 根据que_list题库进行操作

#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <fstream>
#include <unordered_map>
#include "../comm/util.hpp"
#include "../comm/log.hpp"

const string questions_list = "./questions/questions.list";
const string questions_path = "./questions/";

struct Questions
{
    string number; // 题目编号
    string title;  // 题目标题
    string star;   // 题目难度

    int cpu_limit; // 时间要求
    int mem_limit; // 空间要求

    string desc;   // 题目描述
    string header; // 题目预设代码
    string tail;   // 题目测试用例

    string tail_main; //题目测试无用例代码;
};

class Model
{
public:
    Model()
    {
        assert(Load_quest_list(questions_list));
    }
    ~Model()
    {
    }

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
            FileUtil::ReadFile(path + "/header.cpp", q.header, true);
            FileUtil::ReadFile(path + "/tail.cpp", q.tail, true);
            FileUtil::ReadFile(path + "/tail_main.cpp", q.tail_main, true);

            _questions.insert({q.number, q});
        }
        in.close();
        LOG(INFO) << "题库加载完毕" << endl;
        return true;
    }

    // 获取所有题目信息
    bool Get_all_quest(vector<Questions> &out)
    {
        if (_questions.size() == 0)
        {
            LOG(ERROR)<<"获取题库失败"<<endl;
            return false;
        }
        for (auto [key, val] : _questions)
        {
            out.push_back(val);
        }
        return true;
    }

    // 获取指定题目信息
    bool Get_one_quest(const string &quest_number, Questions &out)
    {
        if (_questions.find(quest_number) == _questions.end())
        {
            LOG(ERROR)<<"获取题目失败"<<endl;
            return false;
        }

        out = _questions[quest_number];
        return true;
    }

private:
    // 用哈希表题号来映射题目详细信息
    unordered_map<string, Questions> _questions;
};