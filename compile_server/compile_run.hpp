#pragma once

#include "compiler.hpp"
#include "runner.hpp"
#include "../common/log.hpp"
#include "../common/util.hpp"

#include <jsoncpp/json/json.h>

namespace ns_compiler_and_runner
{
    using namespace ns_util;
    using namespace ns_log;
    using namespace ns_compiler;
    using namespace ns_runner;
    
    class CandR
    {
    public:
        /***********************************
         * in_json: 
         *     "code",          用户提交的代码
         *     "stdin",         用户的输入
         *     "cpu_rlimit",    cpu限制
         *     "mem_rlimit"     内存限制
         * 
         * out_json:
         *     "state",         状态码
         *     "description"    状态码描述
         * 选填:
         *     "stdout"         用户的输出
         *     "stderr"         用户的错误
         * **********************************/
        static void Start(const std::string& in_json, std::string* out_json)
        {
            Json::Reader reader;
            Json::Value in_value;
            reader.parse(in_json, in_value);

            std::string client_code = in_value["code"].asString();
            std::string client_stdin = in_value["stdin"].asString();
            int cpu_rlimit = in_value["cpu_rlimit"].asInt();
            int mem_rlimit = in_value["mem_rlimit"].asInt();

            // 为简便和增加可读性,以下使用了goto语句
            std::string name;
            int state = 0;
            if (client_code.empty())
            {
                state = -1;
                goto END;
            }

            // 获取唯一的文件名
            name = FileUtil::GetUniqName();
            // 将用户提交的代码写入文件
            if (!FileUtil::WriteFile(PathUtil::Src(name), client_code))
            {
                state = -2;
                goto END;
            }

            // 编译用户提交的代码
            if (!Compiler::Compile(name))
            {
                state = -3;
                goto END;
            }

            // 运行
            state = Runner::Run(name, cpu_rlimit, mem_rlimit);
            if (state < 0)
            {
                state = -2;
                goto END;
            }

            // 以上goto全部到这里
            END:
            Json::Value out_value;
            std::string out, err;
            // 读取stdout和stderr的信息
            if (state == 0)
            {
                FileUtil::ReadFile(PathUtil::Stdout(name), &out);
                out_value["stdout"] = out;
            }
            if (state > 0)
            {
                FileUtil::ReadFile(PathUtil::Stderr(name), &err);
                out_value["stderr"] = err;
            }
            else if (state == -3)
            {
                FileUtil::ReadFile(PathUtil::CompileErr(name), &err);
                out_value["stderr"] = err;
            }
            out_value["state"] = state;
            out_value["description"] = GetDescription(state);

            // Json::StyledWriter writer;
            Json::FastWriter writer;
            *out_json = writer.write(out_value);

            RemoveTempFile(name);
            Log(INFO) << "编译运行服务请求成功" << std::endl;
        }
    private:
        static std::string GetDescription(int state)
        {
            std::string msg;
            switch (state)
            {
            case -3:
                msg = "编译错误";
                break;
            case -2:
                msg = "发生了一个错误";
                break;
            case -1:
                msg = "用户提交的代码为空";
                break;
            case 0:
                msg = "运行正常";
                break;
            case SIGABRT:
                msg = "内存超出限制";
                break;
            case SIGFPE:
                msg = "浮点数错误";
                break;
            case SIGXCPU:
                msg = "时间超出限制";
                break;
            default:
                msg = "未知错误";
                break;
            }

            return msg;
        } 
        // 移除临时文件
        static void RemoveTempFile(const std::string& file_name)
        {
            std::string _src = PathUtil::Src(file_name);
            if (FileUtil::IsFileExist(_src))
            {
                unlink(_src.c_str());
            }

            std::string _exe = PathUtil::Exe(file_name);
            if (FileUtil::IsFileExist(_exe))
            {
                unlink(_exe.c_str());
            }

            std::string _cplerr = PathUtil::CompileErr(file_name);
            if (FileUtil::IsFileExist(_cplerr))
            {
                unlink(_cplerr.c_str());
            }

            std::string _stdin = PathUtil::Stdin(file_name);
            if (FileUtil::IsFileExist(_stdin))
            {
                unlink(_stdin.c_str());
            }

            std::string _stdout = PathUtil::Stdout(file_name);
            if (FileUtil::IsFileExist(_stdout))
            {
                unlink(_stdout.c_str());
            }

            std::string _stderr = PathUtil::Stderr(file_name);
            if (FileUtil::IsFileExist(_stderr))
            {
                unlink(_stderr.c_str());
            }
        }
    };
} // namespace ns_compile_and_run
