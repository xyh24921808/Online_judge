#pragma once
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include "../comm/util.hpp"
#include "../comm/log.hpp"
using namespace std;

// 只对代码编译的模块

class Comilper
{
private:
public:
    Comilper()
    {
    }
    ~Comilper()
    {
    }

    // 返回值 true 编译成功 false 编译失败
    // file_name:目标文件名
    static bool Comilp(const string &file_name)
    {

        pid_t pres = fork();
        if (pres < 0)
        {
            LOG(ERROR) << "创建子进程失败" << endl;
            return false;
        }
        else if (pres == 0)
        {
            // 子进程

            // 创建标准错误文件
            umask(0);
            int stderr_ = open(PathUtil::Comilperror(file_name).c_str(), O_CREAT | O_WRONLY, 0644);
            if (stderr_ < 0)
            {
                LOG(WARING) << "没有形成标准错误文件" << endl;
                exit(2);
            }

            // 错误信息重定向到文件中
            dup2(stderr_, 2);

            // 子进程完成编译工作
            execlp("g++","g++",PathUtil::Src(file_name).c_str(),"-D","COMPILER_ONLINE","-o", PathUtil::Erc(file_name).c_str(),nullptr);
            LOG(ERROR) << "启动g++编译器失败" << endl;
            exit(1);
        }
        else
        {
            umask(0);
            // 父进程
            waitpid(pres, nullptr, 0);

            // 编译是否成功 取决是否生成可执行文件
            if (FileUtil::Is_file_exists(PathUtil::Erc(file_name)))
            {
                LOG(INFO) <<PathUtil::Src(file_name)<< "编译成功" << endl;
                return true;
            }
            else
            {
                LOG(INFO) <<PathUtil::Src(file_name)<< "编译失败" << endl;
                return false;
            }
        }
    }
};