#include "comilper_run.hpp"
#include"../comm/rest_rpc/rest_rpc.hpp"
using namespace rest_rpc;
using namespace rest_rpc::rpc_service;
// 线上编译运行模块请求


string comipler_start(rpc_conn conn,const string&in_json)
{
    string out_json;
    if(in_json.size()>=1)
    {
        Comilper_And_Run::Start(in_json,out_json);
        return out_json;
    }
    return "";
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        LOG(ERROR) << "启动命令太少" << endl;
        return -1;
    }

    // Server svr;

    // // svr.Get("/hello", [](const Request &quest, Response &resp)
    // //         { resp.set_content("你好测试", "text/pain;charset=utf-8"); });

    // svr.Post("/comilper_and_run", [](const Request &quest, Response &resp)
    //          {
    //              string in_json = quest.body;
    //              string out_json;
    //              if (in_json.size() >= 1)
    //              {
    //                  Comilper_And_Run::Start(in_json, out_json);
    //                  resp.set_content(out_json, "application/json;charset=utf-8");
    //              }
    //          });

    // svr.listen("0.0.0.0", atoi(argv[1]), 0);

    rpc_server svr(atoi(argv[1]),6);

    svr.register_handler("comilper_and_run",comipler_start);

    svr.run();

    return 0;
}