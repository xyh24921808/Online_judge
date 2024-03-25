//oj_server 采用 M C V 模式

#include"../comm/httplib.h"
#include"oj_control.hpp"
#include<iostream>
using namespace std;
using namespace httplib;


int port=8080;

static Control *ctrl_ptr = nullptr;

void Recovery(int signo)
{
    ctrl_ptr->RecoveryMachine();
}

int main()
{
    signal(SIGQUIT, Recovery);

    //用户请求的服务路由功能
    Server srv;

    Control ctrl;
    ctrl_ptr = &ctrl;


    //请求题库
    srv.Get("/all_questions",[&ctrl](const Request&reqs,Response&reps)
    {
        string html;

        ctrl.All_questsionlist(html);
        reps.set_content(html,"text/html;charset=utf-8");
    });

    //请求题目
    srv.Get(R"(/question/(\d+))",[&ctrl](const Request&reqs,Response&reps)
    {
        string number=reqs.matches[1];
        string html;
        
        ctrl.One_questsion(number,html);
        reps.set_content(html,"text/html;charset=utf-8");
        
    });

    //对题目进行判题
    srv.Post(R"(/judge/(\d+))",[&ctrl](const Request&reqs,Response&reps)
    {
        string number=reqs.matches[1];
        string res_json;
        ctrl.Judge(number,reqs.body,res_json);
        
        reps.set_content(res_json,"application/json;charset=utf-8");
    });


    //对题目进行测试运行 不含判题
    srv.Post(R"(/Testrun/(\d+))",[&ctrl](const Request&reqs,Response&reps)
    {
        string number=reqs.matches[1];
        string res_json;
        ctrl.Testrun(number,reqs.body,res_json);

        reps.set_content(res_json,"application/json;charset=utf-8");
    });

    srv.set_base_dir("./wwwroot/");
    srv.listen("0.0.0.0",port);
    return 0;
}