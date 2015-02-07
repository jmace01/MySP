
#include "Test.h"
#include <sstream>
#include <iostream>

using namespace std;

/******************************************************************
 *
 ******************************************************************/
void Test::testPostfix() {

    bool error = false;
    cout << "Starting ExpressionTreeBuilder Unit Test" << endl;
    clock_t timer = clock();

    TestIO tests[] = {
            TestIO( //Generic complex statement
                    "var += something*5.768+(.03+7) /*What?*/.\"This is a test\" //Hello!",
                    "\"This is a test\" 7 .03 + 5.768 something * + . var +="
            )
            , TestIO( //Unary / Binary mix
                    "4++ + 3--",
                    "3 -- 4 ++ +"
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
			, TestIO( //Operator error
                    "v = 'Hello",
                    "Unterminated String"
            )
            , TestIO( //Parenthesis error
                    "a[a + 2) = 3",
                    "Unexpected closing parenthesis"
            )
            , TestIO( //Parenthesis error
                    "[1] = 2",
                    "Illegal use of '[' without an array"
            )
            , TestIO( //Parenthesis error
                    "1 ` 2",
                    "Unknown operator '`'"
            )
            , TestIO("","")
    };

    ExpressionTreeBuilder* etb = new ExpressionTreeBuilder();
    OperationNode* t;

    string output;

    int i = 0;
    int passed = 0;
    while (tests[i].input != "") {

    	try {
    		t = etb->getExpressionTree(tests[i].input, 1);
    	} catch (PostfixError &e) {
    		if (e.msg != tests[i].output) {
    			error = true;
				cout << "__ERROR______________________" << endl;
				cout << "  INPUT  |  " << tests[i].input << endl;
				cout << "  OUTPUT |  " << e.msg << endl;
				cout << "  EXPECT |  " << tests[i].output << endl;
    		} else {
    		    passed++;
    		}

    		i++;
    		continue;
    	}

        output = t->getPostfix();
        delete t;

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

    float targetSpeed = 0.000079 * i;
    float time = (float)(clock() - timer) / CLOCKS_PER_SEC;
    cout << "    Passed " << passed << "/" << i << " tests in " << time << " seconds" << endl;
    if (time > targetSpeed) cout << "    -- Processed too slow [should be " << targetSpeed << "]" << endl;

    delete etb;

}
