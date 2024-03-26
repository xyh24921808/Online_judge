# 在线编程题库 (Online Judge) 项目
欢迎来到我们的在线编程题库项目！这个项目旨在为用户提供一个方便、高效的平台来解决编程问题，并通过自动化评判系统来评估用户提交的代码。

## 项目简介
我们的在线编程题库项目是一个基于Web的应用程序，旨在让用户能够解决不同难度级别的编程问题。用户可以浏览题目、提交代码，并获得实时的反馈。

## 主要功能
- 题目列表：展示所有可用的编程题目
- 题目详情：每个题目都有详细描述、约束条件等信息。
- 代码编辑器：内置代码编辑器，让用户能够直接在浏览器中编写代码。
- 提交代码：用户可以提交自己的代码解决问题，并即时获得评判结果。
- 评测系统：自动化评测系统能够运行用户提交的代码。

## 项目宏观结构
- comm : 公共工具模块
- compile_server : 编译与运行服务模块
- oj_server : 获取题目列表，查看题目编写题目界面，其他功能


## 技术栈
- 前端:使用html构建用户界面,使用ctemplate渲染页面 如 html/css/js/jquery/ajax
- 后端：采用cpp 多进程 多线程,使用httplib-c++框架处理HTTP请求
- 数据库：采用文件版储存题目信息 [mysql版](https://github.com/xyh24921808/Online_judge/tree/my_sql_model?tab=readme-ov-file)


## 如何使用

## 安装依赖

- [jsoncpp](https://github.com/open-source-parsers/jsoncpp)
- [boostcpp](https://www.boost.org/)
- [ctemplate](https://github.com/OlafvdSpek/ctemplate)
- [MySQL C connect(数据库版存储 见另一个分支)](https://dev.mysql.com/downloads/connector/cpp/)


## 编译
    cd Online_Jugde 
    make all
    make output

## 运行oj_server服务
    cd output/oj_server
    ./oj_server

## 运行comilper_run 服务
    cd output/comilper_server
    ./comilper 端口号