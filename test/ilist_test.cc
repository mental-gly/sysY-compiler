#include "llvm/ADT/ilist.h"
#include "logging.h"
using namespace llvm;

class TestClass : public ilist_node<TestClass> {
public:
    TestClass() = default;
    TestClass(int A) {
        a = A;
    }
    int get() const {
        return a;
    }
private:
    int a;
};

void test() {
    ilist<TestClass> list;
    auto node_1 = new TestClass(1);
    auto node_2 = new TestClass(2);
    auto node_3 = new TestClass(3);
    list.push_back(node_1);
    list.push_back(node_2);
    list.push_back(node_3);
    auto iter = list.begin();
    CHECK_EQ(iter->get(), 1);
    iter++;
    CHECK_EQ(iter->get(), 2);
    iter++;
    CHECK_EQ(iter->get(), 3);
    iter++;
    CHECK_EQ(iter, list.end());
    list.erase(list.begin(), list.end());
}

int main() {
    test();
}
