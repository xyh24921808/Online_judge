#pragma once
// 题库操作模块
// my_sql版本


#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <fstream>
#include <unordered_map>
#include<mysql/mysql.h>
#include "../comm/util.hpp"
#include "../comm/log.hpp"

const string oj_questions="oj_questions";
const string host="127.0.0.1";
const string user="oj_FX";
const string password="xyh123456";
const string db="oj";
const int sql_port=3306;


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
        assert(Load_quest_list());
    }
    ~Model()
    {

    }

    // 加载题目到内存中
    bool Load_quest_list()
    {
        MYSQL*my_ptr=mysql_init(nullptr);

        if(mysql_real_connect(my_ptr,host.c_str(),user.c_str(),password.c_str(),db.c_str(),sql_port,nullptr,0)==nullptr)
        {
            LOG(FATA)<<"致命错误 连接数据库失败"<<endl;
            return false;
        }

        LOG(INFO)<<"连接数据库成功"<<endl;

        string select_sql="SELECT * FROM "+oj_questions;
        if(mysql_query(my_ptr,select_sql.c_str())!=0)
        {
            LOG(WARING)<<"mysql query fate"<<endl;
            return false;
        }

        MYSQL_RES*res=mysql_store_result(my_ptr);

        int row=mysql_num_rows(res);
        int col=mysql_num_fields(res);


        for(int i=0;i<row;i++)
        {
            MYSQL_ROW line=mysql_fetch_row(res);
            Questions q;
            
            q.number=line[0];
            q.title=line[1];
            q.star=line[2];
            q.desc=line[3];

            

            q.header=line[4];
            q.tail=line[5];
            q.tail_main=line[6];
            q.cpu_limit=atoi(line[7]);
            q.mem_limit=atoi(line[8]);

            _questions[q.number]=q;
        }

        

        mysql_close(my_ptr);
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