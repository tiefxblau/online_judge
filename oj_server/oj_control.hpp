#pragma once

#include "oj_model2.hpp"
#include "oj_view.hpp"
#include "../common/httplib.h"

#include <fstream>
#include <cassert>
#include <mutex>
#include <algorithm>
#include <jsoncpp/json/json.h>

namespace ns_control
{
    using namespace ns_model;
    using namespace ns_view;

    const std::string machine_path = "./conf/machine.conf";

    // 提供服务的主机
    struct Machine
    {
        std::string ip;
        int port;
        uint64_t load = 0;  // 机器负载
        int id;
        std::mutex* mtx = nullptr; // std::mutex是防拷贝的, 使用指针以允许Machine进行拷贝

        void IncLoad()
        {
            assert(mtx);
            std::unique_lock<std::mutex> lock(*mtx);

            ++load;
        }
        uint64_t Load()
        {
            assert(mtx);
            std::unique_lock<std::mutex> lock(*mtx);

            return load;
        }
        void DecLoad()
        {
            assert(mtx);
            std::unique_lock<std::mutex> lock(*mtx);

            --load;
        }
        void ResetLoad()
        {
            assert(mtx);
            std::unique_lock<std::mutex> lock(*mtx);

            load = 0;
        }
    };
    class LoadBalance
    {
    private:
        std::vector<Machine> machines; // 全部提供服务的主机, 数组下标对应主机id
        std::vector<int> online;    // 在线主机的id
        std::vector<int> offline;   // 离线主机的id
        std::mutex mtx;
    public:
        LoadBalance()
        {
            assert(InitLB());
        }
        bool InitLB()
        {
            std::ifstream fin(machine_path);
            if (!fin.is_open())
            {
                Log(FATAL) << "未找到配置文件(machine.conf)" << std::endl;
                return false;
            }

            // 加载配置文件中的主机
            std::string line;
            while (std::getline(fin, line))
            {
                std::vector<std::string> split_str;
                StringUtil::Split(&split_str, line, ":");
                if (split_str.size() != 2)
                {
                    Log(ERROR) << "加载某个主机失败, 请检查配置文件" << std::endl;
                    continue;
                }

                Machine m;
                m.id = machines.size();
                m.ip = split_str[0];
                m.port = std::stoi(split_str[1]);
                m.mtx = new std::mutex;

                online.push_back(m.id);
                machines.push_back(std::move(m));
            }

            fin.close();
            Log(INFO) << "加载配置文件" << machine_path << "成功" << std::endl;
            return true;
        }
        bool IntelligentSelect(int* id)
        {
            std::unique_lock<std::mutex> lock(mtx);
            if (online.empty())
            {
                Log(FATAL) << "全部主机均已离线, 请尽快恢复" << std::endl;
                return false;
            }

            // 进行选择
            // 策略: 遍历所有主机,选择负载最小的主机
            int min_load = machines[online[0]].Load();
            *id = online[0];
            for (int i = 0; i < online.size(); ++i)
            {
                uint64_t load = machines[online[i]].Load();
                if (min_load > load)
                {
                    *id = online[i];
                    min_load = load;
                }
            }

            return true;
        }
        Machine& GetMachine(int id)
        {
            std::unique_lock<std::mutex> lock(mtx);
            return machines[id];
        }

        // 所有主机全部上线
        void MachineOnline()
        {
            std::unique_lock<std::mutex> lock(mtx);

            online.insert(online.end(), offline.begin(), offline.end());
            offline.clear();
        }
        // 下线一个主机
        void MachineOffline(int id)
        {
            std::unique_lock<std::mutex> lock(mtx);

            auto it = online.begin();
            while (it != online.end())
            {
                if (*it == id)
                {
                    machines[id].ResetLoad();
                    offline.push_back(id);
                    online.erase(it); // 这里迭代器会失效
                    break;
                }

                ++it;
            }
        }
        // for test
        void ShowState()
        {
            std::unique_lock<std::mutex> lock(mtx);
            
            Log(INFO) << "当前在线主机id列表: ";
            for (auto& id : online)
            {
                std::cout << id << " ";
            }
            std::cout << std::endl;
            Log(INFO) << "当前离线主机id列表: ";
            for (auto& id : offline)
            {
                std::cout << id << " ";
            }
            std::cout << std::endl;
        }
    };

    // 业务核心逻辑
    class Control
    {
    private:
        Model _model; // 负责数据的管理
        View _view;   // 负责渲染html网页
        LoadBalance _load_balance;  //负载均衡器
    public:
        // 获取题目列表网页
        bool GetQuestionBankHtml(std::string* out_html)
        {
            std::vector<Question> qv;
            if (!_model.GetQuestionBank(&qv))
            {
                return false;
            }
            std::sort(qv.begin(), qv.end(), [](const Question& q1, const Question& q2){
                return std::stoi(q1.number) < std::stoi(q2.number);
            });

            return _view.ExpandQuestionBank(qv, out_html);;
        }
        // 获取某个题目的网页
        bool GetQuestionHtml(const std::string& number, std::string* out_html)
        {
            Question q;
            if (!_model.GetOneQuestion(number, &q))
            {
                return false;
            }

            return _view.ExpandQuestion(q, out_html);;
        }
        // 判题
        /**************************
         * in_json:
         *     "code",
         *     "stdin"
         * ************************/
        bool Judge(const std::string& number, const std::string& in_json, std::string* out_json)
        {
            // 获取当前题目的详情信息
            Question q;
            _model.GetOneQuestion(number, &q);

            // std::cout << in_json << std::endl;
            // 反序列化获取传入的信息
            Json::Reader reader;
            Json::Value in_value;
            reader.parse(in_json, in_value);

            Json::Value out_value;
            
            // std::cout << in_value["code"].asString() << std::endl;
            // 构建完整请求以访问编译服务
            out_value["code"] = in_value["code"].asString() + "\n" + q.tail;// 拼接代码,"\n"用来分隔代码连接处 
            out_value["stdin"] = ""; //暂时不处理
            out_value["cpu_rlimit"] = q.cpu_rlimit; 
            out_value["mem_rlimit"] = q.mem_rlimit;

            // 序列化
            Json::FastWriter writer;
            std::string code_jstring = writer.write(out_value);

            while (true)
            {
                int id;
                // 选择负载最低的主机
                if (_load_balance.IntelligentSelect(&id))
                {
                    Machine& m = _load_balance.GetMachine(id);
                    m.IncLoad();

                    // 向编译服务发起请求
                    httplib::Client clt(m.ip, m.port);
                    // 设置超时时间, 保证不会因过多死循环代码导致请求失败离线主机
                    clt.set_read_timeout(10);
                    if (auto res = clt.Post("/compile_run", code_jstring, "application/json;charset=utf-8"))
                    {
                        Log(INFO) << "主机[" << id <<"] 已获取请求 详情: " << m.ip << ":" << m.port << " 当前负载 " << m.Load() << std::endl;
                        m.DecLoad();
                        if (res->status == 200)
                        {
                            *out_json = res->body;
                            break;
                        }
                    }
                    else
                    {
                        // 请求失败,尝试离线对应主机
                        _load_balance.MachineOffline(id);
                        Log(WARNING) << "主机[" << id <<"] 已离线 详情: " << m.ip << ":" << m.port << std::endl;
                        _load_balance.ShowState();
                    }
                }
                else
                {
                    return false;
                }
            }

            return true;
        }
        // 上线离线主机
        void RecoverMachines()
        {
            _load_balance.MachineOnline();
            Log(INFO) << "离线主机重新上线了" << std::endl;
        }
    };
}