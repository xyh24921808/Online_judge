# 在线OJ系统
这是一个简单的在线OJ系统，用于评测用户提交的代码。

## 功能
- 用户可以提交代码解决特定的问题。
- 系统可以自动评测用户提交的代码，并给出反馈。


##  安装与运行
克隆项目到本地:git clone https://github.com/your-username/online-oj.git

支持环境
- Linux Centos5及以上版本


安装依赖:
- [httplib]:[https://github.com/yhirose/cpp-httplib]
- [boostcpp]:[https://www.boost.org/]
- [ctemplate]:[https://github.com/OlafvdSpek/ctemplate]
- [jsoncpp]:[https://github.com/open-source-parsers/jsoncpp]



配置文件信息:
- 题库 ./oj_server/questions/编号/详细信息
- 题目列表 ./oj_server/questions.list
- 编译运行服务 ./oj_server/conf/server_machine.conf


进入项目目录:
cd online_judge


运行应用:
./comilper_server/comilper
./oj_server/oj_server 端口号