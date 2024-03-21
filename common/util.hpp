#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <atomic>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <boost/algorithm/string.hpp>

namespace ns_util
{
    // 默认临时文件存储位置
    const std::string default_path = "./temp/";

    class PathUtil
    {
    public:
        static std::string Src(const std::string& file_name)
        {
            return MakePath(file_name, ".cpp");
        }
        static std::string Exe(const std::string& file_name)
        {
            return MakePath(file_name, ".exe");
        }
        // **********************************************************
        // *用于compile模块
        static std::string CompileErr(const std::string& file_name)
        {
            return MakePath(file_name, ".cplerr");
        }
        // **********************************************************

        static std::string Stdin(const std::string& file_name)
        {
            return MakePath(file_name, ".stdin");
        }
        static std::string Stdout(const std::string& file_name)
        {
            return MakePath(file_name, ".stdout");
        }

        // **********************************************************
        // *用于run模块
        static std::string Stderr(const std::string& file_name)
        {
            return MakePath(file_name, ".stderr");
        }
        // **********************************************************

    private:
        static std::string MakePath(const std::string& file_name, const std::string suffix)
        {
            return default_path + file_name + suffix;
        }
    };

    class TimeUtil
    {
    public:
        static std::string GetTimeStamp()
        {
            timeval time;
            gettimeofday(&time, nullptr);
            return std::to_string(time.tv_sec);
        }
        static std::string GetMillisec()
        {
            timeval time;
            gettimeofday(&time, nullptr);
            uint msec = time.tv_sec * 1000 + time.tv_usec / 1000;
            return std::to_string(msec);
        }
    };
    
    class FileUtil
    {
    public:
        static bool IsFileExist(const std::string& file_path)
        {
            struct stat st;
            return stat(file_path.c_str(), &st) == 0;
        }
        static std::string GetUniqName()
        {
            static std::atomic<uint> cnt(0);
            ++cnt;

            return TimeUtil::GetMillisec() + "_" + std::to_string(cnt);
        }

        static bool WriteFile(const std::string& file_path, const std::string& content)
        {
            std::ofstream fout(file_path);
            if (!fout.is_open())
            {
                return false;
            }

            fout.write(content.c_str(), content.size());
            fout.close();
            return true;
        }
        static bool ReadFile(const std::string& file_path, std::string* content, bool keep = true)
        {
            std::ifstream fin(file_path);
            if (!fin.is_open())
            {
                return false;
            }

            std::string line;
            while (std::getline(fin, line))
            {
                *content += line;
                *content += keep ? "\n" : ""; 
            }

            fin.close();
            return true;
        }
    };

    class StringUtil
    {
    public:
        static void Split(std::vector<std::string>* split_str, const std::string& target, const std::string& sep)
        {
            boost::split(*split_str, target, boost::is_any_of(sep), boost::algorithm::token_compress_on);
        }
    };

} // namespace ns_util
