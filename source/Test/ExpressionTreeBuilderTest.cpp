
#include "Test.h"
#include "../token.h"
#include "../Tokenizer.h"
#include <sstream>
#include <iostream>
#include <queue>

using namespace std;

/****************************************************************************************
 *
 ****************************************************************************************/
void Test::testExTreeBldr() {

    TestIO tests[] = {
            TestIO( //Generic complex statement
                    "var += something*5.768+(.03+7) /*What?*/.\"This is a test\" //Hello!",
                    "\"This is a test\" 7 .03 + 5.768 something * + . var +="
            )
            , TestIO( //Unary / Binary mix
                    "4++ + 3--",
                    "3 -- 4 ++ +"
            )
            , TestIO( //Negative numbers
                    "-10.5 - -.675 - -1",
                    "-1 -.675 - -10.5 -"
            )
            , TestIO( //Keyword statement
                    "print \"Hello \" . \"world!\"",
                    "\"world!\" \"Hello \" . print"
            )
            , TestIO( //Single Operand
                    "1",
                    "1"
            )
            , TestIO( //Array indexing
                    "v = var[x++]",
                    "x ++ var [ v ="
            )
            , TestIO( //Array indexing
                    "var[0][x++]",
                    "x ++ 0 var [ ["
            )
            , TestIO( //Array Indexing
                    "o[5] + 4",
                    "4 5 o [ +"
            )
            , TestIO( //Ternary statement
                    "x = A + ((true == false) ? 1 + 1 : 2 * 2) + B",
                    "B 2 2 * 1 1 + : + false true == ? A + x ="
            )
            , TestIO( //Ternary Error
                    "4 ? 4",
                    "Unfinished ternary statement requires ':' after '?'"
            )
            , TestIO( //Ternary Error
                    "4 : 4 ? 1",
                    "Unexpected ':' with no preceeding '?'"
            )
            , TestIO( //Ternary Error
                    "v = 3 == 4 ? (1 : 2)",
                    "Unexpected ':', expecting ')'"
            )
            , TestIO( //Ternary Error
                    "v = (3 == 4 ?) (1 : 2)",
                    "Unexpected closing parenthesis"
            )
			, TestIO( //Operator error
			        "v = 1 + - 1",
			        "Unexpected Operator -"
			)
			, TestIO( //String error
                    "v = 'Hello",
                    "Unterminated String"
            )
            , TestIO( //Parenthesis error
                    "a[a + 2) = 3",
                    "Unexpected closing parenthesis"
            )
            , TestIO( //Bracket error
                    "[1] = 2",
                    "Illegal use of '[' without an array"
            )
            , TestIO( //Unknown Operator error
                    "1 ` 2",
                    "Unknown operator '`'"
            )
            , TestIO( //Line comment
                    "//Hello",
                    ""
            )
            , TestIO( //Inline comment
                    "/* Test comment */",
                    ""
            )
            , TestIO( //Function call
                    "g(x)",
                    "x P g C"
            )
            , TestIO( //Nested function call
                    "foo(a, bar(b, c, d))",
                    "a P b P c P d P bar C P foo C"
            )
            , TestIO( //Empty function call
                    "foo()",
                    "foo C"
            )
            , TestIO( //Empty function call
                    "v = foo(!a)",
                    "a ! P foo C v ="
            )
            , TestIO( //Missing operand
                    "1 +",
                    "Expecting operand to finish statement"
            )
            , TestIO( //PreUnary operation
                    "!x",
                    "x !"
            )
            , TestIO( //Valid assignment by reference
                    "v = &a",
                    "a & v ="
            )
            , TestIO( //Reference without variable
                    "&1",
                    "Illegal use of '&' without variable"
            )
            , TestIO( //Valid use of keyword
                    "continue",
                    "continue"
            )
            , TestIO( //Valid use of keyword
                    "break",
                    "break"
            )
            , TestIO( //Invalid use of keyword
                    "a = print x",
                    "Unexpected keyword 'print'"
            )
            , TestIO( //Invalid use of keyword
                    "continue x++",
                    "Unexpected keyword 'continue'"
            )
            , TestIO( //Invalid use of keyword
                    "print break",
                    "Unexpected keyword 'break'"
            )
            , TestIO( //Return keyword with value
                    "return true",
                    "true return"
            )
            , TestIO( //Empty return statements
                    "return",
                    "return"
            )
            , TestIO( //Object method call
                    "a->b()",
                    "b a -> C"
            )
            , TestIO( //Static method call
                    "a::b()",
                    "b a :: C"
            )
            , TestIO( //Member variable assignment
                    "a->b = c",
                    "c b a -> ="
            )
            , TestIO( //Static member variable assignment
                    "a::b = c",
                    "c b a :: ="
            )
            , TestIO("","")
    };

    Parser psr = Parser();
    ExpressionTreeBuilder* etb = new ExpressionTreeBuilder();
    OperationNode* t;
    queue<PostfixError> pe;
    Tokenizer tokenizer;

    queue<Token> toks;
    string output;

    int i = 0;
    int passed = 0;
    bool error = false;

    cout << "Starting ExpressionTreeBuilder Unit Test" << endl;
    clock_t timer = clock();

    while (tests[i].input != "") {
        output = "";

    	try {
    	    //Tokenize statement
    	    toks = queue<Token>();
    	    tokenizer.getTokens(tests[i].input, toks);

    	    //Get parser errors
    	    pe = psr.getErrors();
    	    psr.clearErrors();
    	    while (!pe.empty()) {
    	        if (output != "") output += " | ";
    	        output += pe.front().msg;
    	        pe.pop();
    	    }

    	    //Get expression tree
    	    if (output == "") {
                t = etb->getExpressionTree(toks);
                if (t == NULL) {
                    output = "";
                } else {
                    output = t->getPostfix();
                    delete t;
                }
    	    }
    	} catch (PostfixError &e) {
    	    //Get expression tree errors
    		output = e.msg;
    	}



        if (output != tests[i].output) {
            error = true;
            cout << "__ERROR______________________" << endl;
            cout << "  INPUT  |  " << tests[i].input << endl;
            cout << "  OUTPUT |  " << output << endl;
            cout << "  EXPECT |  " << tests[i].output << endl;
        } else {
            passed++;
        }

        i++;
    }

    if (!error) {
        cout << "    No errors in ExpressionTreeBuilder unit test" << endl;
    }

    float targetSpeed = 0.00007 * i;
    float time = (float)(clock() - timer) / CLOCKS_PER_SEC;
    cout << "    Passed " << passed << "/" << i << " tests in " << time << " seconds" << endl;
    if (time > targetSpeed) cout << "    -- Processed too slow [should be " << targetSpeed << "]" << endl;

    delete etb;

}
