#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cassert>
#include <fstream>

#include "../common/log.hpp"
#include "../common/util.hpp"

namespace ns_model
{
    using namespace ns_log;
    using namespace ns_util;

    struct Question
    {
        std::string number; // 题目编号
        std::string title;  // 题目名称
        std::string level;  // 题目难度

        int cpu_rlimit;     // 资源限制(s, kb)
        int mem_rlimit;

        std::string desc;   // 题目描述
        std::string head;   // 提供给用户的默认代码
        std::string tail;   // 补全代码,测试用户编写的接口
    };

    const std::string question_path = "./question_bank/"; // 默认题目文件存储的位置
    const std::string question_list = "./question_bank/question_list";  // 默认题目列表存储的位置
    const std::string SEP = " "; // 题目列表中的分隔符
    class Model
    {
    private:
        std::unordered_map<std::string, Question> questions;
    public:
        Model()
        {
            assert(LoadQuestionBank());
        }

        // 获取全部题目
        bool GetQuestionBank(std::vector<Question>* out)
        {
            if (questions.empty())
            {    
                Log(FATAL) << "获取题库失败" << std::endl;
                return false;
            }

            for (auto& q : questions)
            {
                out->push_back(q.second);
            }

            return true;
        }
        // 获取一个题目
        bool GetOneQuestion(const std::string& number, Question* out)
        {
            auto it = questions.find(number);
            if (it == questions.end())
            {
                Log(WARNING) << "获取题目" << number << "失败" << std::endl;
                return false;
            }

            *out = it->second;
            return true;
        }
    private:
        // 加载题库至内存
        bool LoadQuestionBank()
        {
            std::ifstream fin(question_list);
            if (!fin.is_open())
            {
                Log(FATAL) << "打开题目列表失败" << std::endl;
                return false;
            }

            std::string line;
            while (std::getline(fin, line))
            {
                std::vector<std::string> split_str;
                StringUtil::Split(&split_str, line, SEP);

                if (split_str.size() != 5)
                {
                    continue;
                }

                Question q;
                q.number = split_str[0];
                q.title = split_str[1];
                q.level = split_str[2];
                q.cpu_rlimit = std::stoi(split_str[3]);
                q.mem_rlimit = std::stoi(split_str[4]);

                std::string path = question_path + q.number + "/";
                if (!FileUtil::ReadFile(path + "desc", &q.desc))
                {
                    continue;
                }
                if (!FileUtil::ReadFile(path + "head.cpp", &q.head))
                {
                    continue;
                }
                if (!FileUtil::ReadFile(path + "tail.cpp", &q.tail))
                {
                    continue;
                }

                questions.insert(make_pair(q.number, std::move(q)));
            }

            fin.close();
            Log(Info) << "题库加载成功" << std::endl;
            return true;
        }
    };
} // namespace ns_model
