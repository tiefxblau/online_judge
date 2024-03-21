#pragma once

#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <sys/resource.h>
#include <sys/times.h>
#include <fcntl.h>

#include "../common/log.hpp"
#include "../common/util.hpp"

namespace ns_runner
{
    using namespace ns_log;
    using namespace ns_util;

    class Runner
    {
    public:
        // ****************************************
        // * return value:
        // *     >0: 运行异常,返回收到的信号
        // *     ==0: 运行正常
        // *     <0: 内部错误
        // * 
        // * param:
        // *     cpu_rlimit: 进程最大cpu占用量(秒), 超出会收到信号[24]SIGXCPU
        // *     mem_rlimit: 进程最大内存占用量(kb), 超出会收到信号[6]SIGABRT
        // *
        // ****************************************
        static int Run(const std::string& file_name, int cpu_rlimit, int mem_rlimit)
        {
            umask(0);
            // 在创建子进程之前打开文件,只是为了方便之后的操作
            int stdin_fd = open(PathUtil::Stdin(file_name).c_str(), O_CREAT | O_WRONLY, 0644);
            int stdout_fd = open(PathUtil::Stdout(file_name).c_str(), O_CREAT | O_WRONLY, 0644);
            int stderr_fd = open(PathUtil::Stderr(file_name).c_str(), O_CREAT | O_WRONLY, 0644);

            if (stdin_fd < 0 || stdout_fd < 0 || stderr_fd < 0)
            {
                Log(ERROR) << "创建重定向文件失败" << std::endl;
                return -1;
            }

            pid_t pid = fork();
            if (pid < 0)
            {
                Log(ERROR) << "创建子进程失败" << std::endl;
                return -2;
            }
            else if (pid > 0) // 父进程
            {
                // 先关闭之前打开的文件
                close(stdin_fd);
                close(stdout_fd);
                close(stderr_fd);

                int state;
                waitpid(pid, &state, 0);

                // 返回值只关心收到的信号, 即运行是否出现异常
                return state & 0x7f;
            }
            else // 子进程
            {
                // 设置资源限制
                SetResourceLimit(cpu_rlimit, mem_rlimit);   

                // 将标准输入输出错误全部重定向
                dup2(stdin_fd, 0);
                dup2(stdout_fd, 1);
                dup2(stderr_fd, 2);

                // 运行编译好的文件
                execl(PathUtil::Exe(file_name).c_str(), PathUtil::Exe(file_name).c_str(), nullptr);

                // 不会走到这里
                exit(1);
            }
        }
    private:
        static bool SetResourceLimit(int cpu_rlimit, int mem_rlimit)
        {
            rlimit cpu;
            cpu.rlim_cur = cpu_rlimit;
            cpu.rlim_max = RLIM_INFINITY;

            if (setrlimit(RLIMIT_CPU, &cpu) < 0)
            {
                return false;
            }

            rlimit mem;
            mem.rlim_cur = mem_rlimit * 1024;
            mem.rlim_max = RLIM_INFINITY;

            if(setrlimit(RLIMIT_AS, &mem) < 0)
            {
                return false;
            }
            return true;
        }
    };
} // namespace ns_runner
