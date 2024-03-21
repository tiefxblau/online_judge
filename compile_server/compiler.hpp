#pragma once

#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../common/util.hpp"
#include "../common/log.hpp"

namespace ns_compiler
{
    using namespace ns_util;
    using namespace ns_log;

    class Compiler
    {
    public:
        // ***********************
        // return value:
        // true: 编译成功
        // false: 编译失败
        // ***********************
        static bool Compile(const std::string file_name)
        {
            std::string cplerr = PathUtil::CompileErr(file_name); // 将错误输出到该文件
            std::string src = PathUtil::Src(file_name); // 编译文件
            std::string out = PathUtil::Exe(file_name); // 目标文件

            pid_t pid = fork();
            if (pid < 0)
            {
                Log(ERROR) << "创建子进程失败" << std::endl;
                return false;
            }
            else if (pid > 0) // 父进程
            {
                int state;
                if (waitpid(pid, &state, 0) == pid)
                {
                    // Log(DEBUG) << "wait success" << std::endl;
                }

                // Log(DEBUG) << out << std::endl;
                if (FileUtil::IsFileExist(out)) // 判断目标文件是否存在
                {
                    Log(INFO) << "编译成功" << std::endl;
                    return true;
                }
                else
                {
                    Log(INFO) << "编译失败" << std::endl;
                    return false;
                }
            }
            else // 子进程
            {
                umask(0); // 掩码置0
                int cplerr_fd = open(cplerr.c_str(), O_CREAT | O_WRONLY, 0644);
                if (cplerr_fd < 0)
                {
                    Log(ERROR) << "打开cplerr失败" << std::endl;
                    exit(1);
                }

                // 重定向
                if (dup2(cplerr_fd, 2) < 0)
                {
                    Log(ERROR) << "cplerr重定向失败" << std::endl;
                    exit(2);
                }

                // 进程替换 执行g++编译
                execlp("g++", "g++", "-o", out.c_str(), src.c_str(), "-std=c++11", "-D", "ONLINE_COMPILE", nullptr);

                // 不会走到这里
                exit(3);
            }

            // 不会走到这里
            return false;
        }
    };
} // namespace ns_compiler
