#pragma once
// #define MYSQL_USER_MODEL

// 用户控制模块

#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <jwt-cpp/jwt.h>
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/filters.h>
#include <cryptopp/base64.h>
#include <cassert>
#include <mutex>
#include "../comm/util.hpp"
#include "../comm/log.hpp"
#include "user_template.hpp"

using namespace std;
using namespace CryptoPP;

const string user_path = "./user/";
const string user_file = "users_list.txt";
const string jwt_key_path = "./user/jwt_key.txt";
const string password_key_path = "./user/password_key.txt";

// 用户控制模块
class Users
{
public:
    Users()
    {
        assert(Users_init());
    }
    ~Users()
    {

        ofstream in;
        in.open(user_path + user_file, ios::trunc);
        if (in.is_open() == false)
        {
            LOG(FATA) << "用户数据文件无法打开" << endl;
        }

        for (auto &[key, val] : _user_list)
        {
            string line;
            User_string_line(line, val);
            line += '\n';
            in << line;

            delete val;
        }
        in.close();
    }

public:
    bool Users_init()
    {
        string _key_string;
        if (!FileUtil::ReadFile(password_key_path, _key_string, false) || _key_string.size() != AES::DEFAULT_KEYLENGTH)
        {
            LOG(FATA) << "打开密码密钥文件失败" << endl;
            return false;
        }
        else
        {
            memcpy(_password_key, _key_string.data(), _key_string.size());
        }

        if (FileUtil::ReadFile(jwt_key_path, _jwt_key, false) == false)
        {
            LOG(FATA) << "打开jwt文件失败" << endl;
            return false;
        }

        ifstream in(user_path + user_file);
        if (!in.is_open())
        {
            LOG(FATA) << "用户列表文件打开失败" << endl;
            return false;
        }

        string line;
        while (getline(in, line))
        {
            vector<string> vc;
            StringUtil::String_split(line, " ", vc);

            if (vc.size() == 6)
            {
                // 账号 用户名 密码 邮箱 电话 权限

                User u(vc[0], vc[1], vc[2], vc[3], vc[4]);
                User *ptr = add_user_power(u, vc[5]);

                if (ptr == nullptr)
                {
                    LOG(ERROR) << "用户权限读取失败" << _ID << "行错误" << endl;
                    continue;
                }

                _user_list[_ID] = ptr;
                _user_acc_list[ptr->Get_account()] = _ID;
                _user_name_list[ptr->Get_user_name()] = _ID;
                _ID++;
            }
            else
            {
                LOG(ERROR) << "读取用户" << _ID << "失败" << endl;
            }
        }
        LOG(INFO) << "用户数据加载完成" << endl;

        return true;
    }

    // 增加用户
    bool Add_user(User_attribute &u, const string &power)
    {
        unique_lock<mutex> lock(_mtx);

        // 加密密码
        string hash_password;
        EncryptAES(u._password, hash_password);
        u._password = hash_password;

        User tmp(u._account, u._userName, u._password, u._email, u._phone);
        User *ptr = add_user_power(tmp, power);

        if (ptr == nullptr)
        {
            return false;
        }
        else
        {
            _user_list[_ID] = ptr;
            _user_acc_list[ptr->Get_account()] = _ID;
            _user_name_list[ptr->Get_user_name()] = _ID;
            LOG(INFO) << "添加用户" << _ID << " 权限为" << power << endl;
            _ID++;
            return true;
        }
    }

    // 删除用户 账号方式
    bool Del_user_acc(const string &user_acc)
    {
        unique_lock<mutex> lock(_mtx);
        if (_user_acc_list.find(user_acc) == _user_acc_list.end())
        {
            return false;
        }
        else
        {
            auto id = _user_acc_list[user_acc];
            return Del_id_user(id);
        }
    }

    // 删除用户 用户名方式
    bool Del_user_name(const string &user_name)
    {
        unique_lock<mutex> lock(_mtx);
        if (_user_name_list.find(user_name) == _user_name_list.end())
        {
            return false;
        }
        else
        {
            auto id = _user_name_list[user_name];
            return Del_id_user(id);
        }
    }

    // 查找用户是否存在
    bool Find_user(const string &acc)
    {
        if (_user_acc_list.find(acc) == _user_acc_list.end())
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    // 登录获取jwt接口 token_time:token过期时间 小时
    bool Login_get_jwt(const string &acc, const string &pass, string &out_token, const int &token_time = 48)
    {
        // 对接用户数据否则正确
        auto id = Login_user_has(acc, pass);
        if (id == -1)
        {
            out_token.clear();
            return false;
        }

        User *user_ptr = _user_list[id];

        // 设置 token 过期时间
        auto now = chrono::system_clock::now();
        auto exp = now + chrono::minutes(token_time * 60);

        auto token = jwt::create()
                         .set_issuer("auth0")
                         .set_payload_claim("user_name", jwt::claim(string{user_ptr->Get_user_name()}))
                         .set_payload_claim("account", jwt::claim(string{user_ptr->Get_account()}))
                         .set_expires_at(exp)
                         .sign(jwt::algorithm::hs256{_jwt_key});

        out_token = token;

        return true;
    }

    // 解析jwt返回用户ID
    unsigned int Analysis_jwt(const string &token)
    {
        try
        {
            // 解析token
            auto decoded = jwt::decode(token);

            // 验证器
            auto verifier = jwt::verify().allow_algorithm(jwt::algorithm::hs256{_jwt_key});

            verifier.verify(decoded);

            // 检测token时间是否过期
            if (decoded.has_expires_at() == false)
            {
                LOG(INFO) << "无token时间信息" << endl;
                return -1;
            }
            auto exp = decoded.get_expires_at();
            if (exp < chrono::system_clock::time_point())
            {
                LOG(INFO) << "token时间过期" << endl;
                return -1;
            }

            //寻找账户是否存在
            string account = decoded.get_payload_claim("account").as_string();
            string uset_name = decoded.get_payload_claim("user_name").as_string();

            if (Find_user(account))
            {
                auto id = _user_acc_list[account];
                return id;
            }
            else
            {
                LOG(INFO) << "token信息账户无法找到" << endl;
                return -1;
            }
        }
        catch (const std::exception &e)
        {
            LOG(INFO) << "token验证错误" << e.what();
            return -1;
        }
    }

    // 获取用户
    User *Get_user(unsigned int id)
    {
        if (_user_list.find(id) == _user_list.end())
        {
            return nullptr;
        }
        else
        {
            return _user_list[id];
        }
    }

    // 注册码转权限
    bool Inv_code_topower(const string &code, string &out)
    {
        if (code == "159951")
        {
            out = Regular_u;
            return true;
        }
        else if (code == "173371")
        {
            out = Super_u;
            return true;
        }
        else
        {
            return false;
        }
    }

    void Show_user_list()
    {
        for (auto &[key, val] : _user_list)
        {
            string tmp;
            User_string_line(tmp, val);

            cout << tmp << endl;
        }
    }

private:
    void User_string_line(string &out, User *val, const string &spit = " ")
    {
        string line;
        line += val->Get_account() + " ";
        line += val->Get_user_name() + " ";
        line += val->Get_password() + " ";
        line += val->Get_email() + " ";
        line += val->Get_phone() + " ";
        line += val->Get_mypower();

        out = line;
    }

    // 登录用户是否正确 返回ID
    int Login_user_has(const string &acc, const string &pass)
    {
        unique_lock<mutex> lock(_mtx);
        if (!Find_user(acc))
        {
            return -1;
        }

        unsigned int id = _user_acc_list[acc];
        string out_hashstring;
        string hash_password = _user_list[id]->Get_password();

        DecryptAES(hash_password, out_hashstring);

        if (out_hashstring == pass)
        {
            return id;
        }
        else
        {
            return -1;
        }
    }

    // 为基本模板添加权限
    User *add_user_power(const User &u, const string &power)
    {
        if (_user_name_list[u.Get_user_name()] >= 1 || _user_acc_list[u.Get_account()] >= 1)
        {
            return nullptr;
        }

        if (power == Regular_u)
        {
            Regular_user *ptr = new Regular_user(u);
            return ptr;
        }
        else if (power == Super_u)
        {
            Super_user *ptr = new Super_user(u);
            return ptr;
        }
        else
        {
            return nullptr;
        }
    }

    // 删除用户根据ID
    bool Del_id_user(const unsigned int id)
    {
        if (_user_list.find(id) == _user_list.end())
        {
            return false;
        }
        else
        {
            delete _user_list[id];
            _user_list[id] = nullptr;
            _user_list.erase(id);
            return true;
        }
    }

    // AES加密函数
    bool EncryptAES(const string &plaintext, string &ciphertext)
    {
        // 初始化 AES 加密器
        static CryptoPP::byte iv[AES::BLOCKSIZE] = {'i', 'n', 'i', 't', 'v', 'e', 'c', 't', 'o', 'r', '8', '9', '0', '1', '2', '3'};
        string encrypted_text;
        CBC_Mode<AES>::Encryption encryptor(_password_key, AES::DEFAULT_KEYLENGTH, iv);

        // 执行加密
        StringSource(plaintext, true, new StreamTransformationFilter(encryptor, new StringSink(encrypted_text)));
        StringSource(encrypted_text, true, new Base64Encoder(new StringSink(ciphertext)));

        // 去掉自动添加的换行符
        ciphertext.pop_back();

        return true;
    }

    // AES解密函数
    bool DecryptAES(const string &ciphertext, string &plaintext)
    {
        // 初始化 AES 解密器
        static CryptoPP::byte iv[AES::BLOCKSIZE] = {'i', 'n', 'i', 't', 'v', 'e', 'c', 't', 'o', 'r', '8', '9', '0', '1', '2', '3'};
        string decrypted_text;
        string decoded_text;

        // 解码 Base64 编码的密文
        StringSource(ciphertext, true, new Base64Decoder(new StringSink(decoded_text)));

        // 执行解密
        CBC_Mode<AES>::Decryption decryptor(_password_key, AES::DEFAULT_KEYLENGTH, iv);
        StringSource(decoded_text, true, new StreamTransformationFilter(decryptor, new StringSink(decrypted_text)));

        plaintext = decrypted_text;

        return true;
    }

private:
    mutex _mtx;
    unsigned int _ID = 1;
    string _jwt_key;                                      // jwt密钥字符串
    CryptoPP::byte _password_key[AES::DEFAULT_KEYLENGTH]; // 密码密钥字符串

    unordered_map<unsigned int, User *> _user_list;
    unordered_map<string, unsigned int> _user_name_list;
    unordered_map<string, unsigned int> _user_acc_list;
};