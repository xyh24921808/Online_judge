// http/https 请求控制库
#pragma once
#include "../comm/httplib.h"
#include "oj_control.hpp"

using namespace std;
using namespace httplib;

#define USER_INDEX_HTML_PATH "./user/user_login.html"       // 用户登录界面
#define USER_REGISTER_HTML_PATH "./user/user_register.html" // 用户登录界面

#define ROOT_HTML_PATH "./wwwroot/index.html"
class Http_control
{
public:
    //请求主页 Get
    static void Get_root_index_get(const Request &reqs, Response &reps)
    {
        string html;
        if(FileUtil::ReadFile(ROOT_HTML_PATH,html,true))
        {
            reps.set_content(html, "text/html;charset=utf-8");
        }
        else 
        {
            reps.status = 404;
            reps.set_content("404 Not Found", "text/plain");
        }
    }

    // 请求题库 Get
    static void Get_all_questions_get(const Request &reqs, Response &reps)
    {
        string html;

        // 带搜索词
        if (reqs.has_param("word"))
        {

            string word = reqs.get_param_value("word");

            LOG(INFO) << "用户搜索题库:" << word << endl;
            _ctrl->All_search_questsionlist(word, html);
        }
        else
        {
            _ctrl->All_questsionlist(html);
        }
        reps.set_content(html, "text/html;charset=utf-8");
    }

    // 请求题目 Get
    static void Get_question_get(const Request &reqs, Response &reps)
    {
        string html;
        string number = reqs.matches[1];

        _ctrl->One_questsion(number, html);
        reps.set_content(html, "text/html;charset=utf-8");
    }

    // 请求题目修改信息页面 Get
    static void Get_question_modify_info_html_get(const Request &reqs, Response &reps)
    {
        // // 验证token
        // unsigned int id = 0;
        // if (!Token_validate(reqs, reps, id))
        // {
        //     return;
        // }

        // // 验证权限
        // User *u_ptr = _ctrl->Get_user(id);
        // if (u_ptr->Delete_question_power() == false)
        // {
        //     reps.body = "用户权限不足";
        //     return;
        // }

        string html;
        string number = reqs.matches[1];

        _ctrl->Questsion_info_html(number, html);
        reps.set_content(html, "text/html;charset=utf-8");
    }

    // 请求题目修改代码页面 Get
    static void Get_question_modify_code_htmt_get(const Request &reqs, Response &reps)
    {
        // // 验证token
        // unsigned int id = 0;
        // if (!Token_validate(reqs, reps, id))
        // {
        //     return;
        // }

        // // 验证权限
        // if ((_ctrl->Get_user(id)->Modfiy_question_power()) == false)
        // {
        //     reps.body = "用户权限不足";
        //     return;
        // }

        string html;
        string number = reqs.matches[1];

        _ctrl->Questsion_code_html(number, html);
        reps.set_content(html, "text/html;charset=utf-8");
    }

    // 拉起用户登录界面 Get
    static void Get_user_login_index_get(const Request &reqs, Response &reps)
    {
        string html;
        FileUtil::ReadFile(USER_INDEX_HTML_PATH, html, true);
        reps.set_content(html, "text/html;charset=utf-8");
    }

    // 拉起用户注册界面 Get
    static void Get_user_register_index_get(const Request &reqs, Response &reps)
    {
        string html;
        FileUtil::ReadFile(USER_REGISTER_HTML_PATH, html, true);
        reps.set_content(html, "text/html;charset=utf-8");
    }

    // 用户登录
    static void Get_user_login_post(const Request &reqs, Response &reps)
    {

        string in_json = reqs.body;
        string res_json;
        _ctrl->Login(in_json, res_json);

        Json::Value root;
        Json::Reader reader;
        reader.parse(res_json, root);

        int status = root["status"].asInt();
        string token = root["reason"].asString();

        if (status == 1)
        {
            reps.set_header("Authorization", token);
        }

        reps.set_content(res_json, "application/json;charset=utf-8");
    }

    // 用户注册
    static void Get_user_register_post(const Request &reqs, Response &reps)
    {
        string in_json = reqs.body;
        string res_json;
        _ctrl->Register_user(in_json, res_json);
        reps.set_content(res_json, "application/json;charset=utf-8");
    }

    // 判题 post
    static void Jugde_question_post(const Request &reqs, Response &reps)
    {
        unsigned int id = 0;
        if (Token_validate(reqs, reps, id))
        {
            string number = reqs.matches[1];
            string res_json;

            User *u_ptr = _ctrl->Get_user(id);

            _ctrl->Judge(number, reqs.body, res_json);
            LOG(INFO) << u_ptr->Get_account() << "用户进行" << number << "号提交任务"
                      << " " << u_ptr->Get_mypower() << endl;
            reps.set_content(res_json, "application/json;charset=utf-8");
        }
    }

    // 测试运行 post
    static void Testrun_question_post(const Request &reqs, Response &reps)
    {
        unsigned int id = 0;
        if (Token_validate(reqs, reps, id))
        {
            string number = reqs.matches[1];
            string res_json;

            User *u_ptr = _ctrl->Get_user(id);

            _ctrl->Testrun(number, reqs.body, res_json);
            LOG(INFO) << u_ptr->Get_account() << "用户进行" << number << "号测试运行任务"
                      << " " << u_ptr->Get_mypower() << endl;
            reps.set_content(res_json, "application/json;charset=utf-8");
        }
    }

    // 删除题目 post
    static void Del_question(const Request &reqs, Response &reps)
    {
        // 验证token
        unsigned int id = 0;
        if (!Token_validate(reqs, reps, id))
        {
            return;
        }

        string res_json;
        User *u_ptr = _ctrl->Get_user(id); // 获取用户

        if (u_ptr->Delete_question_power()) // 检测用户权限
        {
            string number = reqs.matches[1];
            _ctrl->Question_del(reqs.body, res_json);

            LOG(INFO) << "用户" << u_ptr->Get_account() << " 进行删除题目" << number << "操作" << endl;
        }
        else
        {
            Json::Value out_val;
            out_val["status"] = -1;
            out_val["reason"] = "用户无权限删除";
            res_json = StringUtil::JsonToString(out_val);

            LOG(INFO) << "用户" << u_ptr->Get_account() << " 尝试删除题目失败 无权限" << endl;
        }

        reps.set_content(res_json, "application/json;charset=utf-8");
    }

    // 提交修改题目文本 post
    static void Modify_question_info(const Request &reqs, Response &reps)
    {
        // 验证token
        unsigned int id = 0;
        if (!Token_validate(reqs, reps, id))
        {
            return;
        }

        string res_json;
        User *u_ptr = _ctrl->Get_user(id); // 获取用户

        if (u_ptr->Modfiy_question_power()) // 检测用户权限
        {
            string number = reqs.matches[1];
            _ctrl->Question_modify_info(number, reqs.body, res_json);

            LOG(INFO) << "用户" << u_ptr->Get_account() << " 进行修改题目" << number << "的信息" << "操作" << endl;
        }
        else
        {
            Json::Value out_val;
            out_val["status"] = -1;
            out_val["reason"] = "用户无权限修改";
            res_json = StringUtil::JsonToString(out_val);

            LOG(INFO) << "用户" << u_ptr->Get_account() << " 修改题目信息失败 无权限" << endl;
        }
        reps.set_content(res_json, "application/json;charset=utf-8");
    }

    // 提交修改题目代码 post
    static void Modify_question_code(const Request &reqs, Response &reps)
    {
        // 验证token
        unsigned int id = 0;
        if (!Token_validate(reqs, reps, id))
        {
            return;
        }

        string res_json;
        User *u_ptr = _ctrl->Get_user(id); // 获取用户

        if (u_ptr->Modfiy_question_power()) // 检测用户权限
        {
            string number = reqs.matches[1];
            _ctrl->Question_modify_code(number, reqs.body, res_json);

            LOG(INFO) << "用户" << u_ptr->Get_account() << " 进行修改题目" << number << "的代码" << "操作" << endl;
        }
        else
        {
            Json::Value out_val;
            out_val["status"] = -1;
            out_val["reason"] = "用户无权限修改";
            res_json = StringUtil::JsonToString(out_val);

            LOG(INFO) << "用户" << u_ptr->Get_account() << " 修改题目代码失败 无权限" << endl;
        }
        reps.set_content(res_json, "application/json;charset=utf-8");
    }

private:
    // 检测https请求是否有token而且合法
    static bool Token_validate(const Request &reqs, Response &reps, unsigned int &out_id)
    {
        auto auth_header = reqs.get_header_value("Authorization");

        if (auth_header.find("Bearer ") != std::string::npos)
        {
            // 提取 token
            auto token = auth_header.substr(7); // 去除 "Bearer "

            auto id = _ctrl->Verify_token(token);
            if (id != -1)
            {
                out_id = id;
                return true;
            }
            else
            {
                reps.set_content("Token_not_find", "text/plain");
                reps.status = 401;
                return false;
            }
        }
        else
        {
            reps.set_content("未登录", "text/plain");
            reps.status = 401;
            return false;
        }
    }

public:
    static Control *_ctrl;

private:
};