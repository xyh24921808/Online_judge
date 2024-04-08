#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <fstream>
#include "../comm/util.hpp"
#include "../comm/log.hpp"
using namespace std;

const string user_path = "./user/";
const string user_file = "users_list.txt";

// 用户模块
class User
{
public:
    User(const string &acc, const string &name, const string &pass, const string &email = "", const string &phone = "")
    {
        _account = acc;
        _userName = name;
        _password = pass;
        _email = email;
        _phone = phone;
    }
    ~User()
    {
    }

public:
    virtual string Get_account()
    {
        return _account;
    }
    virtual string Get_user_name()
    {
        return _userName;
    }
    virtual string Get_email()
    {
        return _email;
    }
    virtual string Get_phone()
    {
        return _phone;
    }
    virtual string Get_password()
    {
        return _password;
    }

private:
    string _account;
    string _password;
    string _email;
    string _phone;
    string _userName;
};

class Super_User : public User
{
public:
private:
};

// 用户控制模块
class Users_control
{
public:
    Users_control()
    {
        Users_init();
    }
    ~Users_control()
    {

        ofstream in;
        in.open(user_path + user_file, ios::trunc);

        for (auto &[key, val] : _userlist)
        {
            string line;
            User_addString(line, val);
            in << line;

            delete val;
        }
    }

public:
    bool Users_init()
    {
        ifstream in(user_path + user_file);
        if (!in.is_open())
        {
            LOG(FATA) << "用户列表文件打开失败" << endl;
            return false;
        }

        string line;
        unsigned int ID = 0;
        while (getline(in, line))
        {
            vector<string> vc;
            StringUtil::String_split(line, " ", vc);

            // 分词大小6是超级用户 反之普通用户
            if (vc.size() == 5)
            {
                User *ptr = new User(vc[0], vc[1], vc[2], vc[3], vc[4]);
                _userlist[ID] = ptr;
            }
            else if (vc.size() == 6)
            {
                ////////////////////////
            }
            else
            {
                LOG(ERROR) << "读取用户行失败" << endl;
            }
            ID++;
        }
    }

    void Show_user_list()
    {
        for (auto &[key, val] : _userlist)
        {
            string tmp;
            User_addString(tmp, val);

            cout << tmp << endl;
        }
    }

private:
    void User_addString(string &out, User *val, const string &spit = " ")
    {
        string line;
        line += val->Get_user_name() + " ";
        line += val->Get_account() + " ";
        line += val->Get_password() + " ";
        line += val->Get_email() + " ";
        line += val->Get_phone();
        out = line;
    }

private:
    unordered_map<unsigned int, User *> _userlist;
};