#include "compile_run.hpp"
#include "../common/httplib.h"

using namespace ns_compiler_and_runner;
const std::string WWWROOT = "./wwwroot";

static void Usage(char* proc)
{
    std::cout << "Usage:\n\t" << proc << " port" << std::endl;
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        Usage(argv[0]);
        exit(1);
    }
    int port = atoi(argv[1]);

    httplib::Server svr;

    // 请求编译服务
    svr.Post("/compile_run", [](const httplib::Request& req, httplib::Response& resp){
        std::string json_req = req.body;
        if (!json_req.empty())
        {
            std::string json_resp;
            CandR::Start(json_req, &json_resp);

            resp.set_content(json_resp, "application/json;charset=utf-8");
        }
    });

    svr.listen("0.0.0.0", port);
    return 0;
}