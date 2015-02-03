
#include "Test.h"
#include <sstream>
#include <iostream>

using namespace std;

/******************************************************************
 *
 ******************************************************************/
void Test::testPostfix() {

    bool error = false;
    cout << "Starting Postfix Unit Test" << endl;
    clock_t timer = clock();

    TestIO tests[] = {
            TestIO( //Generic complex statement
                    "var += something*5.768+(.03+7) /*What?*/.\"This is a test\" //Hello!",
                    "5.768 something * 7 .03 + @1 @0 + \"This is a test\" @2 . @3 var += @4 "
            )
            , TestIO( //Unary / Binary mix
                    "4++ + 3--",
                    "4 ++ 3 -- @1 @0 + @2 "
            )
            , TestIO( //Keyword statement
                    "print \"Hello \" . \"world!\"",
                    "\"world!\" \"Hello \" . @0 print "
            )
            , TestIO( //Single Operand
                    "1",
                    "1 "
            )
            , TestIO( //Array indexing
                    "v = var[x++]",
                    "x ++ @0 var [ @1 v = @2 "
            )
            , TestIO( //Array indexing
                    "var[0][x++]",
                    "0 var [ x ++ @1 @0 [ @2 "
            )
            , TestIO( //Array Indexing
                    "o[5] + 4",
                    "5 o [ 4 @0 + @1 "
            )
            , TestIO( //Ternary statement
                    "x = A + ((true == false) ? 1 + 1 : 2 * 2) + B",
                    "false true == @0 ? 1 1 + 2 2 * @2 @1 : B @3 + @4 A + @5 x = @6 "
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
            , TestIO("","")
    };

    Postfix* p = new Postfix();
    OperationNode* t;

    string output;

    int i = 0;
    int passed = 0;
    while (tests[i].input != "") {

    	try {
    		t = p->getPostfix(tests[i].input, 1);
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
        cout << "    No errors in Postfix unit test" << endl;
    }

    float targetSpeed = 0.000079 * i;
    float time = (float)(clock() - timer) / CLOCKS_PER_SEC;
    cout << "    Passed " << passed << "/" << i << " tests in " << time << " seconds" << endl;
    if (time > targetSpeed) cout << "    -- Processed too slow [should be " << targetSpeed << "]" << endl;

    delete p;

}
