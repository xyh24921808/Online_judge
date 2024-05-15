#pragma once

// 用户模板
#include <iostream>
#include <string>

#define Super_u "Super"     // 超级用户
#define Regular_u "Regular" // 普通用户

// 用户属性
struct User_attribute
{
    string _account;
    string _userName;
    string _password;
    string _email;
    string _phone;
};

// 基本用户模板
class User
{
public:
    User(const string &acc, const string &name, const string &pass, const string &email = "", const string &phone = "")
    {
        _user._account = acc;
        _user._userName = name;
        _user._password = pass;
        _user._email = email;
        _user._phone = phone;
    }
    virtual string Get_account() const
    {
        return _user._account;
    }
    virtual string Get_user_name() const
    {
        return _user._userName;
    }
    virtual string Get_email() const
    {
        return _user._email;
    }
    virtual string Get_phone() const
    {
        return _user._phone;
    }
    virtual string Get_password() const
    {
        return _user._password;
    }

    virtual string Get_mypower() const
    {
        return "unkown_power";
    }

    virtual bool Add_question_power()
    {
        return false;
    }
    virtual bool Delete_question_power()
    {
        return false;
    }
    virtual bool Modfiy_question_power()
    {
        return false;
    }

protected:
    User_attribute _user;
};

// 普通用户
class Regular_user : public User
{
public:
    Regular_user(const User &u)
        : User(u)
    {
    }
    virtual ~Regular_user()
    {
    }

public:
    virtual string Get_mypower() const
    {
        return "Regular";
    }

private:
};

// 超级用户
class Super_user : public User
{
public:
    Super_user(const User &u)
        : User(u)
    {
    }
    virtual ~Super_user()
    {
    }

public:
    virtual string Get_mypower() const
    {
        return "Super";
    }
    virtual bool Add_question_power()
    {
        return true;
    }

    virtual bool Delete_question_power()
    {
        return true;
    }

    virtual bool Modfiy_question_power()
    {
        return true;
    }

private:
};