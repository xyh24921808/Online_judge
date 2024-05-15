#pragma once
// 数据构建视图模块

#include <vector>
#include <string>
#include <iostream>
#include <ctemplate/template.h>
#include <unordered_map>

#include "../oj_user/user_template.hpp"

using namespace std;

const string template_path = "./template_html/";

class Veiw
{
public:
    // 根据题库生成html网页
    void All_explandhtml(vector<Questions> &all, string &html)
    {
        string src_html = template_path + "all_question.html";

        ctemplate::TemplateDictionary root("all_question");

        sort(all.begin(), all.end(), [](Questions &a, Questions &b) -> bool
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
            } });

        for (const auto &q : all)
        {
            ctemplate::TemplateDictionary *sub = root.AddSectionDictionary("question_list");
            sub->SetValue("number", q.number);
            sub->SetValue("title", q.title);
            sub->SetValue("star", q.star);

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
        ptr->Expand(&html, &root);
    }

    // 根据单个题目生成html网页
    void One_explandhtml(Questions &one, string &html, const progra_lage &type = CPP)
    {
        string src_html = template_path + "one_question.html";

        ctemplate::TemplateDictionary root("one_question");
        root.SetValue("number", one.number);
        root.SetValue("title", one.title);
        root.SetValue("star", one.star);
        root.SetValue("desc", one.desc);

        string header;
        one.Get_progar_header(type, header);
        root.SetValue("pre_code", header);

        ctemplate::Template *ptr = ctemplate::Template::GetTemplate(src_html, ctemplate::DO_NOT_STRIP);
        ptr->Expand(&html, &root);
    }

    // 修改题目描述html
    // 根据题目信息生成html网页
    void Quest_info_explandhtml(Questions &one, string &html)
    {
        string src_html = template_path + "info_question.html";
        ctemplate::TemplateDictionary root("info_question");
        root.SetValue("number", one.number);
        root.SetValue("title", one.title);
        root.SetValue("star", one.star);
        root.SetValue("desc", one.desc);

        ctemplate::Template *ptr = ctemplate::Template::GetTemplate(src_html, ctemplate::DO_NOT_STRIP);
        ptr->Expand(&html, &root);
    }

    // 修改题目代码html
    // 根据题目代码内容生成html网页
    void Quest_code_explandhtml(Questions &one, string &html, const progra_lage &type = CPP)
    {
        string src_html = template_path + "code_question.html";

        string header, tail, test_run;
        one.Get_progar_all(type, header, tail, test_run);

        ctemplate::TemplateDictionary root("code_question");
        root.SetValue("number", one.number);
        root.SetValue("header", header);
        root.SetValue("tail", tail);
        root.SetValue("test_run", test_run);

        ctemplate::Template *ptr = ctemplate::Template::GetTemplate(src_html, ctemplate::DO_NOT_STRIP);
        ptr->Expand(&html, &root);
    }

private:
};