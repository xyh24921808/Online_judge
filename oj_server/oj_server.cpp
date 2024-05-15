// oj_server 采用 M C V 模式

// #define CPPHTTPLIB_OPENSSL_SUPPORT //https

#include "./oj_https.hpp"

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
#include <openssl/ssl.h>
#include <openssl/crypto.h>
#include <openssl/opensslv.h>
#define SERVER_CERT_FILE "./ssl/crt.crt"        // 证书路径
#define SERVER_PRIVATE_KEY_FILE "./ssl/key.key" // 私钥路径
#endif

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
int port = 443;
#endif

#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
int port = 80;
#endif

static Control *ctrl_ptr = nullptr;
Control *Http_control::_ctrl = nullptr;

const string pre_http_path = "";

void Recovery(int signo)
{
    if (signo == SIGINT)
    {
        ctrl_ptr->~Control();
        exit(0);
    }
}

int main(int argnum, char *argvs[])
{
    signal(SIGINT, Recovery);

    if (argnum == 2)
    {
        port = atoi(argvs[1]);
    }

// https
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
    SSLServer srv(SERVER_CERT_FILE, SERVER_PRIVATE_KEY_FILE);
#endif

// http
#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
    Server srv;
#endif

    Control ctrl;
    ctrl_ptr = &ctrl;
    Http_control::_ctrl = &ctrl;

    // 拉取题库
    srv.Get(pre_http_path + "/all_questions", Http_control::Get_all_questions_get);

    // 拉取题目
    srv.Get(pre_http_path + R"(/question/(\d+))", Http_control::Get_question_get);

    // 用户登录界面
    srv.Get(pre_http_path + "/user_index", Http_control::Get_user_login_index_get);

    // 拉取用户注册界面
    srv.Get(pre_http_path + "/register_index", Http_control::Get_user_register_index_get);

    // 拉取题目修改信息页面
    srv.Get(pre_http_path + R"(/question_modify_info_index/(\d+))", Http_control::Get_question_modify_info_html_get);

    // 拉取题目修改信息页面
    srv.Get(pre_http_path + R"(/question_modify_code_index/(\d+))", Http_control::Get_question_modify_code_htmt_get);

    // 用户登录
    srv.Post(pre_http_path + "/login", Http_control::Get_user_login_post);

    // 用户注册
    srv.Post(pre_http_path + "/register", Http_control::Get_user_register_post);

    // 判题
    srv.Post(pre_http_path + R"(/judge/(\d+))", Http_control::Jugde_question_post);

    // 测试运行
    srv.Post(pre_http_path + R"(/Testrun/(\d+))", Http_control::Testrun_question_post);

    // 删除题目
    srv.Post(pre_http_path + R"(/del_question/(\d+))", Http_control::Del_question);

    // 修改题目文本
    srv.Post(pre_http_path + R"(/modify_question_info/(\d+))", Http_control::Modify_question_info);

    // 修改题目代码
    srv.Post(pre_http_path + R"(/modify_question_code/(\d+))", Http_control::Modify_question_code);

    srv.set_base_dir("./wwwroot/");

    srv.listen("0.0.0.0", port);

    return 0;
}