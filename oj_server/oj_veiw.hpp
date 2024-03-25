#pragma once
// 数据构建视图模块

#include <vector>
#include <string>
#include <ctemplate/template.h>
#include <iostream>

#include "oj_model.hpp"

using namespace std;


const string template_path="./template_html/";


// struct Questions
// {
//     string number; // 题目编号
//     string title;  // 题目标题
//     string star;   // 题目难度

//     int cpu_limit; // 时间要求
//     int mem_limit; // 空间要求

//     string desc;   // 题目描述
//     string header; // 题目预设代码
//     string tail;   // 题目测试用例
// };

class Veiw
{
public:
    // 根据题库生成html网页
    void All_explandhtml(vector<Questions> &all, string &html)
    {
        string src_html=template_path+"all_question.html";


        ctemplate::TemplateDictionary root("all_question");

        sort(all.begin(),all.end(),[](Questions&a,Questions&b)->bool
        {
            int n1=stoi(a.number);
            int n2=stoi(b.number);

            if(n1==n2)
            {
                return CmpUtil::Starcmp(a.star,b.star);
            }
            else 
            {
                return n1<n2;
            }
        });

        for(const auto&q:all)
        {
            ctemplate::TemplateDictionary*sub=root.AddSectionDictionary("question_list");
            sub->SetValue("number",q.number);
            sub->SetValue("title",q.title);
            sub->SetValue("star",q.star);

            if (q.star == "简单")
            {
                sub->SetFormattedValue("difficulty", "easy");
            }
            else if (q.star == "中等")
            {
                sub->SetFormattedValue("difficulty", "medium");
            }
            else if (q.star == "困难")
            {
                sub->SetFormattedValue("difficulty", "hard");
            }
        }
        
        ctemplate::Template *ptr = ctemplate::Template::GetTemplate(src_html, ctemplate::DO_NOT_STRIP);
        ptr->Expand(&html,&root);

    }

    // 根据单个题目生成html网页
    void One_explandhtml(const Questions &one, string &html)
    {
        string src_html=template_path+"one_question.html";

        ctemplate::TemplateDictionary root("one_question");
        root.SetValue("number",one.number);
        root.SetValue("title",one.title);
        root.SetValue("star",one.star);
        root.SetValue("desc",one.desc);
        root.SetValue("pre_code",one.header);

        ctemplate::Template *ptr = ctemplate::Template::GetTemplate(src_html, ctemplate::DO_NOT_STRIP);
        ptr->Expand(&html,&root);
    }

private:
};