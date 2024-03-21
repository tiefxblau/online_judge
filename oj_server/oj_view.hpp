#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <ctemplate/template.h>

#include "oj_model2.hpp"

namespace ns_view
{
    using namespace ns_model;

    // 被渲染网页的路径
    const std::string template_path = "./template_html/";

    class View
    {
    public:
        bool ExpandQuestionBank(const std::vector<Question>& questons, std::string* out_html)
        {
            // 建立ctemplate参数目录结构
            ctemplate::TemplateDictionary root("question_list");
            for (auto& q : questons)
            {
                // 子目录,同时对应html模板中questionlist.html中的{{#question}}循环
                ctemplate::TemplateDictionary* sec = root.AddSectionDictionary("question");
                // 向结构中添加要替换的数据
                sec->SetValue("number", q.number);
                sec->SetValue("title", q.title);
                sec->SetValue("level", q.level);
            }

            // 开始渲染，返回新的网页结果到out_html
            // DO_NOT_STRIP：保持html网页原貌
            return ctemplate::ExpandTemplate(template_path + "questionlist.html", ctemplate::DO_NOT_STRIP, &root, out_html);
        }
        bool ExpandQuestion(const Question& question, std::string* out_html)
        {
            ctemplate::TemplateDictionary root("question");
            root.SetValue("number", question.number);
            root.SetValue("title", question.title);
            root.SetValue("level", question.level);
            root.SetValue("desc", question.desc);
            root.SetValue("head", question.head);

            return ctemplate::ExpandTemplate(template_path + "question.html", ctemplate::DO_NOT_STRIP, &root, out_html);
        }
    };

}