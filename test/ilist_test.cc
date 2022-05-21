#include "llvm/ADT/ilist.h"
#include "ADT/ilist.h"
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

// would not fit the ilist_node trait if not
// inherit from ilist_node
class Derived : public TestClass,
                public ilist_node<Derived>
{
public:
    explicit Derived(int a) : TestClass(a) {}
};

class PointerClass {
public:
    int a;
};

class ast_class : public ast_ilist_node<ast_class> {
public:
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

    // derived
    ilist<Derived> list2;
    auto node_4 = new Derived(4);
    list2.push_back(node_4);
    auto iter2 = list2.begin();
    CHECK_EQ(iter2->get(), 4);


// store pointer, dangerous!
//    ilist<PointerClass *> list3;
//    auto node_5 = new PointerClass;
//    node_5->a = 5;
//    list3.push_back(&node_5);

    // testing our easy ilist
    auto node_6 = new ast_class;
    auto node_7 = new ast_class;
    node_6->a = 6;
    node_7->a = 7;
    node_6->Prev = node_7;
    node_7->Next = node_6;
    CHECK_EQ(node_7->Next->a, 6);
}

int main() {
    test();
}
