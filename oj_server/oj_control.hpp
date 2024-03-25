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
#include "oj_model.hpp"
#include "oj_veiw.hpp"
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

    // 用户提交代码模块
    void Judge(const string &number, const string &in_json, string &out_json)
    {

        string comilper_string;
        // 第四个参数是否保留测试用例
        func(number, in_json, comilper_string, true);

        // 4.使用负载均衡模块获取服务器id
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
            // 5.对服务器主机提交https

            httplib::Client cli(mac->Get_ip(), mac->Get_port());
            mac->Inc_load();
            if (auto res = cli.Post("/comilper_and_run", comilper_string, "application/json;charset=utf-8"))
            {
                if (res.value().status == 200)
                {
                    // 6.返回结果给上层
                    out_json = res.value().body;
                    mac->Dec_load();

                    LOG(INFO) << "请求编译运行服务成功" << endl;
                    break;
                }
                else
                {
                    mac->Dec_load();
                }
            }
            else
            {
                _blance.Off_machine(id);
                LOG(ERROR) << "请求主机失败:" << id << " "
                           << "信息:" << mac->Get_ip() << ":" << mac->Get_port() << "可能已经离线" << endl;

                //_blance.Show_allmachines();
            }
        }
    }

    // 用户测试代码模块
    void Testrun(const string &number, const string &in_json, string &out_json)
    {

        string comilper_string;
        // 第四个参数是否保留测试用例
        func(number, in_json, comilper_string, false);

        // 4.使用负载均衡模块获取服务器id
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
            // 5.对服务器主机提交https

            httplib::Client cli(mac->Get_ip(), mac->Get_port());
            mac->Inc_load();
            if (auto res = cli.Post("/comilper_and_run", comilper_string, "application/json;charset=utf-8"))
            {
                if (res.value().status == 200)
                {
                    // 6.返回结果给上层
                    out_json = res.value().body;
                    mac->Dec_load();

                    LOG(INFO) << "请求编译运行服务成功" << endl;
                    break;
                }
                else
                {
                    mac->Dec_load();
                }
            }
            else
            {
                _blance.Off_machine(id);
                LOG(ERROR) << "请求主机失败:" << id << " "
                           << "信息:" << mac->Get_ip() << ":" << mac->Get_port() << "可能已经离线" << endl;

                //_blance.Show_allmachines();
            }
        }
    }

    // 上线所有离线主机
    void RecoveryMachine()
    {
        _blance.On_all_machines();
    }

private:
    // 内部函数
    void func(const string &number, const string &in_json, string &out_json, bool is_tail)
    {
        // 1.根据number获取题号
        Questions q;
        _model.Get_one_quest(number, q);

        // 2.对in_json反序列化得到用户的代码的标准输入
        Json::Reader reader;
        Json::Value in_value;
        reader.parse(in_json, in_value);
        string code = in_value["code"].asString();

        // 3.对题号的tail测试用例和用户提交的代码进行合并 or 不添加测试用例
        Json::Value comilper_val;
        comilper_val["input"] = in_value["input"].asString();

        if (is_tail)
            comilper_val["code"] = code + '\n' + q.tail; // 添加测试用例
        else
            comilper_val["code"] = code + '\n' + q.tail_main; // 直接对用户的代码进行编译运行 不合并tail.cpp

        comilper_val["cpu_limit"] = q.cpu_limit;
        comilper_val["mem_limit"] = q.mem_limit;

        out_json = StringUtil::JsonToString(comilper_val);
    }

private:
    Model _model;        // 构建数据
    Veiw _veiw;          // 网页渲染
    Load_blance _blance; // 负载均衡器
};