#include <iostream>
#include <signal.h>

#include "../common/httplib.h"
#include "oj_control.hpp"

using namespace httplib;
using namespace ns_control;

Control* ctrl_ptr;

void Recovery(int signal)
{
    ctrl_ptr->RecoverMachines();
}

int main()
{
    // 注册SIGQUIT信号: 恢复离线主机
    signal(SIGQUIT, Recovery);
    //用户请求的服务路由功能
    Server svr;

    Control ctrl;
    ctrl_ptr = &ctrl;

    // 获取所有的题目列表
    svr.Get("/questions", [&ctrl](const Request &req, Response &resp){
        //返回一张包含有所有题目的html网页
        std::string html;
        ctrl.GetQuestionBankHtml(&html);
        resp.set_content(html, "text/html; charset=utf-8");
    });

    // 用户要根据题目编号，获取题目的内容
    // /question/100 -> 正则匹配
    // R"()", 原始字符串raw string,保持字符串内容的原貌，不用做相关的转义
    svr.Get(R"(/questions/(\d+))", [&ctrl](const Request &req, Response &resp){
        std::string number = req.matches[1];
        std::string html;
        ctrl.GetQuestionHtml(number, &html);
        resp.set_content(html, "text/html; charset=utf-8");
    });

    // 用户提交代码，使用我们的判题功能(1. 每道题的测试用例 2. compile_and_run)
    svr.Post(R"(/judge/(\d+))", [&ctrl](const Request &req, Response &resp){
        std::string number = req.matches[1];
        std::string result_json;
        ctrl.Judge(number, req.body, &result_json);
        resp.set_content(result_json, "application/json;charset=utf-8");
        // resp.set_content("指定题目的判题: " + number, "text/plain; charset=utf-8");
    });
    
    svr.set_base_dir("./wwwroot");
    svr.listen("0.0.0.0", 8080);
    return 0;
}