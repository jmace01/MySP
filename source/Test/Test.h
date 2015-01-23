#ifndef SOURCE_TEST_TEST_H_
#define SOURCE_TEST_TEST_H_

#include "../postfix.h";
#include <string>


struct TestIO {
    TestIO(std::string i, std::string o) {
        this->input  = i;
        this->output = o;
    }
    std::string input;
    std::string output;
};


class Test {
    public:

        Test() {
            Test::testPostfix();
        }

        static void testPostfix();
};

#endif
