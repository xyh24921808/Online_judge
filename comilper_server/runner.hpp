#pragma once

#include "../comm/util.hpp"
#include "../comm/log.hpp"
#include <iostream>
#include <string>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
using namespace std;

// 运行模块

class Runner
{
public:
    Runner()
    {
    }
    ~Runner()
    {
    }

    // 返回程序运行结果的信号
    // cpu_limit 最大运行时长 单位=秒
    // mem_limit 最大内存大小 单位=KB
    static int Run(const string &file_name, int cpu_limit, int mem_limit)
    {

        // 把程序运行的结果标准错误呵标准输出重定向到之地文件
        // 标准输入不考虑
        string _execute = PathUtil::Erc(file_name);
        string _stdin = PathUtil::Stdin(file_name);
        string _stdout = PathUtil::Stdout(file_name);
        string _stderr = PathUtil::Stdrun_err(file_name);

        umask(0);
        int _stdin_fd = open(_stdin.c_str(), O_RDONLY | O_CREAT, 0644);
        int _stdout_fd = open(_stdout.c_str(), O_WRONLY | O_CREAT, 0644);
        int _stderr_fd = open(_stderr.c_str(), O_WRONLY | O_CREAT, 0644);

        if (FileUtil::Is_file_exists(_execute) == false)
        {
            LOG(ERROR) << "找不到程序文件" << endl;
            return -3;
        }
        if (_stderr_fd < 0 || _stdin_fd < 0 || _stdout_fd < 0)
        {
            LOG(ERROR) << "文件打开失败" << endl;
            return -1;
        }

        pid_t pid = fork();
        if (pid < 0)
        {
            close(_stderr_fd);
            close(_stdin_fd);
            close(_stdout_fd);
            LOG(ERROR) << "创建子进程失败" << endl;
            return -2;
        }
        else if (pid == 0)
        {
            // 子进程

            // 标准输入，输出，错误 重定向
            dup2(_stdin_fd, 0);
            dup2(_stdout_fd, 1);
            dup2(_stderr_fd, 2);

            // 设置资源约束 防止超时/内存过大
            Setproclimit(cpu_limit, mem_limit);

            // 直接执行程序
            execl(_execute.c_str(), _execute.c_str(), nullptr);
            exit(1);
        }
        else
        {
            // 父进程
            close(_stdin_fd);
            close(_stdout_fd);
            close(_stderr_fd);

            int status = 0;
            waitpid(pid, &status, 0);
            LOG(INFO) << file_name << "程序运行完毕" << endl;

            return status & 0x7F;
        }
    }

private:
    // 内部函数 设置程序运行大小和内存大小
    static void Setproclimit(int _cpu, int _mem)
    {
        struct rlimit cpu_rlimit;
        struct rlimit mem_rlimit;

        cpu_rlimit.rlim_max = RLIM_INFINITY;
        cpu_rlimit.rlim_cur = _cpu;
        setrlimit(RLIMIT_CPU, &cpu_rlimit);

        mem_rlimit.rlim_cur = _mem * 1024;
        mem_rlimit.rlim_max = RLIM_INFINITY;
        setrlimit(RLIMIT_AS, &mem_rlimit);
    }
};