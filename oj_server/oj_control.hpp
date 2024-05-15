#pragma once
// 控制模块

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <cassert>
#include <fstream>
#include <set>
#include "../comm/httplib.h"
#include "../comm/log.hpp"
#include "../oj_user/user_control.hpp"
#include "oj_model.hpp"
#include "oj_veiw.hpp"

#include "../comm/rest_rpc/rest_rpc.hpp"
using namespace rest_rpc;
using namespace rest_rpc::rpc_service;
using namespace std;

// 主机属性
class Machine
{
public:
    Machine(const string &ip = "", const int port = 0)
    {
        _load_count = 0;
        _lock = new mutex;

        _ip = ip;
        _port = port;
    }
    ~Machine()
    {
    }

    string Get_ip()
    {
        return _ip;
    }
    int Get_port()
    {
        return _port;
    }
    uint64_t Get_load()
    {
        return _load_count;
    }

public:
    // 增加负载
    void Inc_load()
    {
        if (_lock == nullptr)
            return;

        _lock->lock();
        _load_count++;
        _lock->unlock();
    }

    // 减少负载
    void Dec_load()
    {
        if (_lock == nullptr)
            return;

        _lock->lock();
        _load_count--;
        _lock->unlock();
    }

private:
    string _ip;           // 主机ip
    int _port;            // 主机端口
    uint64_t _load_count; // 负载数量
    mutex *_lock = nullptr;
};

const string machines_path = "./conf/server_machine.conf";

/// 负载均衡模块
class Load_blance
{
public:
    Load_blance()
    {
        assert(Load_machines(machines_path));
        LOG(INFO) << "加载" << machines_path << "成功" << endl;
    }
    ~Load_blance()
    {
    }

public:
    // 加载服务器列表配置
    bool Load_machines(const string &path)
    {
        fstream in(path);
        if (in.is_open() == false)
        {
            LOG(FATA) << "致命错误 读取服务器配置文件失败" << endl;
            return false;
        }

        string line;
        while (getline(in, line))
        {
            vector<string> tokens;
            StringUtil::String_split(line, ":", tokens);

            if (tokens.size() != 2)
            {
                LOG(WARING) << "切分服务器配置:" << line << "失败" << endl;
                continue;
            }

            Machine m(tokens[0], stoi(tokens[1]));

            _online.insert(_machines.size()); // 插入machines主机下标
            _machines.push_back(m);
        }

        in.close();
        return true;
    }

    // 获得最小的负载主机
    bool Smart_choice(int &id, Machine **out_mac)
    {
        _mtx.lock();

        int oline_size = _online.size();

        if (oline_size == 0)
        {
            _mtx.unlock();
            LOG(FATA) << "致命错误 所有主机离线" << endl;
            return false;
        }

        id = *(_online.begin());
        uint64_t min_load = _machines[id].Get_load();
        for (auto &mac_id : _online)
        {
            auto load = _machines[mac_id].Get_load();
            if (min_load > load)
            {
                id = mac_id;
                min_load = load;
            }
        }

        *out_mac = &_machines[id];
        _mtx.unlock();
        return true;
    }

    // 上线某台主机
    void Off_machine(int id)
    {
        _mtx.lock();
        _online.erase(id);
        _offline.insert(id);
        _mtx.unlock();
    }

    // 离线某台主机
    void On_machine(int id)
    {
        _mtx.lock();
        _online.insert(id);
        _offline.erase(id);
        _mtx.unlock();
    }

    // 上线全部主机
    void On_all_machines()
    {
        _mtx.lock();
        for (auto &x : _offline)
        {
            _online.insert(x);
        }
        LOG(INFO) << "所有主机已上线" << endl;
        _mtx.unlock();
    }

    // 离线全部主机
    void Off_all_machines()
    {
        _mtx.lock();
        for (auto &x : _online)
        {
            _offline.insert(x);
        }
        LOG(INFO) << "所有主机已离线" << endl;
        _mtx.unlock();
    }

    // 打印主机列表
    void Show_allmachines()
    {
        _mtx.lock();
        cout << "当前在线主机列表" << endl;
        for (auto &id : _online)
        {
            cout << _machines[id].Get_ip() << ":" << _machines[id].Get_port() << endl;
        }

        cout << "当离线主机列表" << endl;
        for (auto &id : _offline)
        {
            cout << _machines[id].Get_ip() << ":" << _machines[id].Get_port() << endl;
        }
        _mtx.unlock();
    }

private:
    vector<Machine> _machines; // 可提供服务所有主机
    set<int> _online;          // 在线主机
    set<int> _offline;         // 离线主机
    mutex _mtx;
};

/// 控制器核心模块
class Control
{
public:
    Control()
    {
    }
    ~Control()
    {
    }

public:
    // 根据题库构建网页
    bool All_questsionlist(string &html)
    {
        vector<Questions> all;
        if (_model.Get_all_quest(all))
        {
            _veiw.All_explandhtml(all, html);
            return true;
        }
        else
        {
            html = "获取题库失败";
            return false;
        }
    }

    // 根据搜索词构建题库网页
    bool All_search_questsionlist(const string &word, string &html)
    {
        vector<Questions> all;
        if (_model.Get_search_questlist(word, all))
        {
            _veiw.All_explandhtml(all, html);
            return true;
        }
        else
        {
            html = "获取题库失败";
            return false;
        }
    }

    // 根据题号构建网页
    bool One_questsion(const string &number, string &html)
    {
        Questions one;
        if (_model.Get_one_quest(number, one))
        {
            _veiw.One_explandhtml(one, html);
            return true;
        }
        else
        {
            html = number + "题目不存在";
            return false;
        }
    }

    // 根据题目信息构建题目修改页面
    bool Questsion_info_html(const string &number, string &html)
    {
        Questions one;
        if (_model.Get_one_quest(number, one))
        {
            _veiw.Quest_info_explandhtml(one, html);
            return true;
        }
        else
        {
            html = number + "题目不存在";
            return false;
        }
    }

    // 根据题目代码构建题目修改页面
    bool Questsion_code_html(const string &number, string &html)
    {
        Questions one;
        if (_model.Get_one_quest(number, one))
        {
            _veiw.Quest_code_explandhtml(one, html);
            return true;
        }
        else
        {
            html = number + "题目不存在";
            return false;
        }
    }

    // 提交代码模块
    void Judge(const string &number, const string &in_json, string &out_json)
    {

        string comilper_string;
        Merge_code(number, in_json, comilper_string, true);

        // 使用负载均衡模块获取服务器id
        Smart_choies_mac(number, comilper_string, in_json, out_json);
    }

    // 测试代码模块
    void Testrun(const string &number, const string &in_json, string &out_json)
    {

        string comilper_string;
        Merge_code(number, in_json, comilper_string, false);

        // 使用负载均衡模块获取服务器id
        Smart_choies_mac(number, comilper_string, in_json, out_json);
    }

    // 修改题目信息模块
    //  json格式
    ////////////////////////////////////////
    // 输入:
    // title:标题
    // desc:描述
    // star:难度
    ///////////////////////////////////////
    // 输出:
    // status:执行状态
    // reason:结果
    ////////////////////////////////////////
    void Question_modify_info(const string &number, const string &in_json, string &out_json)
    {
        Json::Value in_val;
        Json::Value out_val;
        Json::Reader reader;

        reader.parse(in_json, in_val);

        string title = in_val["title"].asString();
        string desc = in_val["desc"].asString();
        string star = in_val["star"].asString();

        int status = 0;
        string reason;

        if (_model.Modify_quest_info(number, title, desc, star))
        {
            status = 1;
            reason = "修改成功";
        }
        else
        {
            status = -1;
            reason = "修改失败 题目不存在或内部错误";
        }

        out_val["status"] = status;
        out_val["reason"] = reason;

        out_json = StringUtil::JsonToString(out_val);
    }

    ////////////////////////////////////////
    // 输入:
    // language:编程语言
    // header:预设代码
    // tail:测试用例
    // test_main:测试运行主体
    ///////////////////////////////////////
    // 输出:
    // status:执行状态
    // reason:结果
    ////////////////////////////////////////
    // 修改题目代码模块
    void Question_modify_code(const string &number, const string &in_json, string &out_json)
    {
        Json::Value in_val;
        Json::Value out_val;
        Json::Reader reader;

        reader.parse(in_json, in_val);

        string lag_string = in_val["language"].asString();
        string header = in_val["header"].asString();
        string tail = in_val["tail"].asString();
        string test_main = in_val["test_main"].asString();

        int status = 0;
        string reason;
        progra_lage lag;
        if (lag_string == "CPP" || lag_string == "c++" || lag_string == "cpp")
        {
            lag = CPP;
        }
        else
        {
            status = -1;
            reason = "提交编程语言错误 无法找到对应的语言";
            goto END;
        }

        if (_model.Modify_quest_code(number, lag, header, tail, test_main))
        {
            status = 1;
            reason = "修改代码成功";
        }
        else
        {
            status = -2;
            reason = "修改代码失败 题目无法找到 或未知错误";
        }
    END:
        out_val["status"] = status;
        out_val["reason"] = reason;
        out_json = StringUtil::JsonToString(out_val);
    }

    // 删除题目模块
    // json格式
    ////////////////////////////////////////
    // 输入:
    // number:题号
    ///////////////////////////////////////
    // 输出:
    // status:执行状态
    // reason:结果
    ////////////////////////////////////////
    void Question_del(const string &in_json, string &out_json)
    {
        // 反序列化json 得到题号
        Json::Value out_val;
        Json::Value in_val;
        Json::Reader reader;
        reader.parse(in_json, in_val);

        string number = in_val["number"].asString();

        if (_model.Del_quest(number))
        {
            out_val["status"] = 1;
            out_val["reason"] = "删除题目成功";
        }
        else
        {
            out_val["status"] = -1;
            out_val["reason"] = "删除失败 题目未找到";
        }

        out_json = StringUtil::JsonToString(out_val);
    }

    // 用户登录模块
    // 登录核心
    //  json格式
    ////////////////////////////////////////
    // 输入:
    // account:账号
    // password:密码
    //
    // 输出:
    // status:执行状态
    // reason:结果
    ////////////////////////////////////////
    void Login(const string &in_json, string &out_json)
    {
        // 对in_json进行反序列化
        Json::Reader reader;
        Json::Value in_val;
        Json::Value out_val;
        reader.parse(in_json, in_val);

        string account = in_val["account"].asString();
        string password = in_val["password"].asString();

        string token;
        // 进行登录验证返回token
        if (_users.Login_get_jwt(account, password, token) == false)
        {
            out_val["status"] = -1;
            out_val["reason"] = "用户名或密码错误";
            out_json = StringUtil::JsonToString(out_val);

            LOG(INFO) << account << "账户登录失败" << endl;
        }
        else
        {
            out_val["status"] = 1;
            out_val["reason"] = token;
            out_json = StringUtil::JsonToString(out_val);

            LOG(INFO) << account << "账户登录成功 已发送token" << endl;
        }
    }

    // 用户注册模块
    // json格式
    ////////////////////////////////////////
    // 输入:
    // account:账号
    // password:密码
    // username:用户名
    // email:邮箱
    // phone:电话
    // invcode:注册码
    // 输出:
    // status:执行状态
    // reason:结果
    ////////////////////////////////////////
    void Register_user(const string &in_json, string &out_json)
    {
        Json::Reader reader;
        Json::Value in_val;
        Json::Value out_val;
        reader.parse(in_json, in_val);

        string acc = in_val["account"].asString();
        string name = in_val["username"].asString();
        string pass = in_val["password"].asString();
        string email = in_val["email"].asString();
        string phone = in_val["phone"].asString();
        string inv_code = in_val["invcode"].asString();
        User_attribute U_bute;

        int status = 0;
        string reason;
        string power;

        // 根据注册码获取权限
        if (_users.Inv_code_topower(inv_code, power) == false)
        {
            status = -1;
            reason = "注册码错误";
            goto END;
        }

        // 用户名是否存在
        if (_users.Find_user(acc))
        {
            status = -2;
            reason = "账号已存在";
            goto END;
        }

        U_bute._account = acc;
        U_bute._userName = name;
        U_bute._password = pass;
        U_bute._email = email;
        U_bute._phone = phone;
        if (_users.Add_user(U_bute, power) == false)
        {
            status = -3;
            reason = "内部错误无法注册用户";
            goto END;
        }
        else
        {
            status = 1;
            reason = "注册成功";
            goto END;
        }

    END:
        LOG(INFO) << "执行注册任务 "
                  << "状态:" << status << " 结果:" << reason << endl;
        out_val["status"] = status;
        out_val["reason"] = reason;
        out_json = StringUtil::JsonToString(out_val);
    }

    // 验证token
    unsigned int Verify_token(const string &token)
    {
        return _users.Analysis_jwt(token);
    }

    // id获取用户
    User *Get_user(unsigned int id)
    {
        return _users.Get_user(id);
    }

    // 上线所有离线主机
    void RecoveryMachine()
    {
        _blance.On_all_machines();
    }

private:
    // 内部函数
    // Type:加载那种编程语言类型
    // is_tail:是否保留测试用例
    void Merge_code(const string &number, const string &in_json, string &out_json, bool is_tail)
    {
        // 1.根据number获取题号
        Questions q;
        _model.Get_one_quest(number, q);

        // 2.对in_json反序列化得到用户的代码的标准输入
        Json::Reader reader;
        Json::Value in_value;
        reader.parse(in_json, in_value);

        string code = in_value["code"].asString();
        string lag_string = in_value["language"].asString();

        // 3.获取代码编程语言
        progra_lage lag;
        Prog_language_Util::lang_string_to(lag_string, lag);

        // 4.对题号的tail测试用例和用户提交的代码进行合并 or 不添加测试用例
        Json::Value comilper_val;
        comilper_val["input"] = in_value["input"].asString();

        if (is_tail)
        {
            // 添加测试用例
            string tail;
            q.Get_progar_tail(lag, tail);
            comilper_val["code"] = code + '\n' + tail;
        }
        else
        {
            // 直接对用户的代码进行编译运行 不合并
            string test_main;
            q.Get_progar_test_main(lag, test_main);
            comilper_val["code"] = code + '\n' + test_main;
        }

        comilper_val["cpu_limit"] = q.cpu_limit;
        comilper_val["mem_limit"] = q.mem_limit;
        comilper_val["language"] = lag_string;

        out_json = StringUtil::JsonToString(comilper_val);
    }

    // 内部函数
    void Smart_choies_mac(const string &number, const string &comilper_string, const string &in_json, string &out_json)
    {
        while (1)
        {
            int id = 0;
            Machine *mac = nullptr;
            if (_blance.Smart_choice(id, &mac) == false)
            {
                break;
            }
            LOG(INFO) << "选择主机成功:" << id << " "
                      << "信息:" << mac->Get_ip() << ":" << mac->Get_port() << endl;

            // 5.对服务器主机提交
            rpc_client cli(mac->Get_ip(), mac->Get_port());
            bool has_cont = cli.connect(2);
            if (has_cont)
            {
                mac->Inc_load();
                string res = cli.call<string>("comilper_and_run", comilper_string);
                mac->Dec_load();

                out_json = res;
                LOG(INFO) << "请求编译运行服务成功" << endl;

                break;
            }
            else
            {
                _blance.Off_machine(id);
                LOG(ERROR) << "请求主机失败:" << id << " "
                           << "信息:" << mac->Get_ip() << ":" << mac->Get_port() << "可能已经离线" << endl;
            }
        }
    }

private:
    Model _model;        // 题库数据
    Veiw _veiw;          // 网页渲染
    Users _users;        // 用户数据
    Load_blance _blance; // 负载均衡器
};