#pragma once
#include "comilper.hpp"
#include "runner.hpp"
#include <json/json.h>

// 编译 运行 合并模块

class Comilper_And_Run
{

public:
    // json格式
    ////////////////////////////////////////
    // 输入:
    // input:用户的输入
    // code:用户的代码
    // cpu_limit:程序时间约束
    // mem_limit:程序空间约束
    //
    // 输出:
    // reason:执行结果
    // status:状态码
    // stdout:运行完毕输出结果
    // stderr:运行完错误结果
    ////////////////////////////////////////
    static void Start(const string &in_json, string &out_json)
    {
        Json::Value in_val;
        Json::Value out_val;
        Json::Reader Rer;

        // 反序列化输入json
        Rer.parse(in_json, in_val);

        string code = in_val["code"].asString();
        string input = in_val["input"].asString();
        int cpu_limit = in_val["cpu_limit"].asInt();
        int mem_limit = in_val["mem_limit"].asInt();

        int code_status = 0;
        int run_res = 0;

        // 形成唯一文件名
        string file_name = FileUtil::Uniqe_file_name();

        // 如果代码为空
        if (code.size() == 0)
        {
            code_status = -1;
            goto END;
        }

        // 向唯一文件写入code
        if (FileUtil::WrtieFile(PathUtil::Src(file_name), code) == false)
        {
            code_status = -2;
            goto END;
        }
        
        //向唯一输入文件写入input
        if(FileUtil::WrtieFile(PathUtil::Stdin(file_name),input)==false)
        {
            code_status=-1;
            goto END;
        }
        
        // 编译code
        if (Comilper::Comilp(file_name) == false)
        {
            // 读取编译错误结果
            code_status = -3;
            goto END;
        }

        // 运行code
        run_res = Runner::Run(file_name, cpu_limit, mem_limit);
        if (run_res < 0)
        {
            // 内部没有运行
            code_status = -2;
        }
        else if (run_res > 0)
        {
            // 运行失败
            code_status = run_res;
        }
        else
        {
            // 运行成功
            code_status = 0;
        }

    END:
        out_val["stauts"] = code_status;
        out_val["reason"] = Codeto_str(code_status, file_name);

        if (code_status == 0)
        {
            string tmp_out;

            // 读取程序运行之后的标准输出
            FileUtil::ReadFile(PathUtil::Stdout(file_name), tmp_out, true);
            out_val["stdout"] = tmp_out;

            // 读取程序运行之后的标准错误
            FileUtil::ReadFile(PathUtil::Stdrun_err(file_name), tmp_out, true);
            out_val["stderr"] = tmp_out;
        }

        // 反序列化结果
        Json::StyledWriter writer;
        
        //out_json = writer.write(out_val);
        out_json=StringUtil::JsonToString(out_val);
        Remove_temp(file_name);
    }

    // 清理临时文件
    void static Remove_temp(const string &file_name)
    {
        string path_name = PathUtil::Src(file_name);
        if (FileUtil::Is_file_exists(path_name))
            unlink(path_name.c_str());

        string Erc_name = PathUtil::Erc(file_name);
        if (FileUtil::Is_file_exists(Erc_name))
            unlink(Erc_name.c_str());

        string com_name = PathUtil::Comilperror(file_name);
        if (FileUtil::Is_file_exists(com_name))
            unlink(com_name.c_str());

        string input_name = PathUtil::Stdin(file_name);
        if (FileUtil::Is_file_exists(input_name))
            unlink(input_name.c_str());

        string output_name = PathUtil::Stdout(file_name);
        if (FileUtil::Is_file_exists(output_name))
            unlink(output_name.c_str());

        string err_name = PathUtil::Stdrun_err(file_name);
        if (FileUtil::Is_file_exists(err_name))
            unlink(err_name.c_str());
    }

private:
    // 内部信号转换函数
    static string Codeto_str(int code, const string &file_name)
    {
        string desc;
        switch (code)
        {
        case 0:
            desc = "编译运行成功";
            break;
        case -1:
            desc = "用户提交代码为空";
            break;
        case -2:
            desc = "内部未知错误";
            break;
        case -3:
            // 编译错误
            FileUtil::ReadFile(PathUtil::Comilperror(file_name), desc, true);
            break;
        case SIGABRT:
            desc = "内存超出范围";
            break;
        case SIGXCPU:
            desc = "运行时间超时";
            break;
        case SIGSEGV:
            desc = "出现段错误";
            break;
        default:
            desc = "ukowndebug:" + to_string(code);
            break;
        }
        return desc;
    }
};