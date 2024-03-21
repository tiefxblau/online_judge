#include "oj_model.hpp"

using namespace ns_model;

int main()
{
    Model model;

    std::vector<Question> qv;
    model.GetQuestionBank(&qv);
    for (auto& q : qv)
    {
        std::cout << q.number << q.title << q.level << std::endl;
    }

    return 0;
}