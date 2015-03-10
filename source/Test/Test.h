#ifndef SOURCE_TEST_TEST_H_
#define SOURCE_TEST_TEST_H_

#include <string>
#include "../Parser.h"
#include "../ExpressionTreeBuilder.h"


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
            Test::testExTreeBldr();
            Test::testParser();
            //Test::testVariables();
        }

        static void testExTreeBldr();
        static void testParser();
        static void testVariables();
};

#endif
