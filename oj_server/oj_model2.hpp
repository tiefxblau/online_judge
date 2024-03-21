#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cassert>
#include <fstream>
#include <mysql/mysql.h>

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

    const std::string questions_table_name = "questions";
    const std::string mysql_host = "101.43.183.8";
    const std::string mysql_user = "oj_client";
    const std::string user_password = "OJCYy3142536";
    const std::string questions_db = "oj_questions";
    const uint mysql_port = 3556;
    class Model
    {
    public:
        Model()
        {}

        bool QueryMySQL(const std::string& query, std::vector<Question>* out)
        {
            MYSQL* mysql = mysql_init(nullptr);
            mysql_set_character_set(mysql, "utf8");

            if (mysql_real_connect(mysql, mysql_host.c_str(), mysql_user.c_str(), \
                user_password.c_str(), questions_db.c_str(), mysql_port, nullptr, 0) == nullptr)
            {
                Log(FATAL) << "连接数据库失败" << std::endl;
                return false;
            }
            Log(INFO) << "连接数据库成功" << std::endl;

            if (mysql_query(mysql, query.c_str()) != 0)
            {
                Log(ERROR) << query << "执行失败" << std::endl;
                return false;
            }

            MYSQL_RES* res = mysql_store_result(mysql);

            my_ulonglong rows = mysql_num_rows(res);
            uint cols = mysql_num_fields(res);

            for (int i = 0; i < rows; ++i)
            {
                MYSQL_ROW line = mysql_fetch_row(res);
                Question q;
                q.number = line[0];
                q.title = line[1];
                q.level = line[2];
                q.cpu_rlimit = atoi(line[3]);
                q.mem_rlimit = atoi(line[4]);
                q.desc = line[5];
                q.head = line[6];
                q.tail = line[7];

                out->push_back(std::move(q));
            }

            free(res);

            mysql_close(mysql);
            return true;
        }
        // 获取全部题目
        bool GetQuestionBank(std::vector<Question>* out)
        {
            std::string query = "select * from ";
            query += questions_table_name;

            if (!QueryMySQL(query, out))
            {
                Log(FATAL) << "获取题库失败" << std::endl;
                return false;
            }
            return true;
        }
        // 获取一个题目
        bool GetOneQuestion(const std::string& number, Question* out)
        {
            std::string query = "select * from ";
            query += questions_table_name;
            query += " where id=";
            query += number;
            std::vector<Question> questions;

            if (!QueryMySQL(query, &questions))
            {
                Log(WARNING) << "获取题目" << number << "失败" << std::endl;
                return false;
            }

            *out = questions[0];
            return true;
        }
    
    };
} // namespace ns_model
