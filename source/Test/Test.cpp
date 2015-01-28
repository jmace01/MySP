
#include "Test.h"
#include <sstream>
#include <iostream>

using namespace std;

/******************************************************************
 *
 ******************************************************************/
void Test::testPostfix() {

    bool error = false;

    TestIO tests[] = {
              TestIO(
                    "var += something*5.768+(.03+7) /*What?*/.\"This is a test\" //Hello!",
                    "5.768 something * 7 .03 + @1 @0 + \"This is a test\" @2 . @3 var += @4 "
              )
            , TestIO(
                    "4++ + 3--",
                    "4 ++ 3 -- @1 @0 + @2 "
              )
            , TestIO(
                    "print \"Hello \" . \"world!\"",
                    "\"world!\" \"Hello \" . @0 print "
              )
            , TestIO(
                    "1",
                    "1 "
              )
            , TestIO(
                    "v = var[x++]",
                    "x ++ @0 var [ @1 v = @2 "
              )
            , TestIO(
                    "x = A + ((true == false) ? 1 + 1 : 2 * 2) + B",
                    "false true == @0 ? 1 1 + 2 2 * @2 @1 : B @3 + @4 A + @5 x = @6 "
              )
            , TestIO(
                    "v = 1 + function(2+7*12, \"string\".\"string2\")",
                    "12 7 * @0 2 + @1 , \"string2\" \"string\" . @2 , function @CALL @3 1 + @4 v = @5 "
              )
            , TestIO("","")
    };

    Postfix* p = new Postfix();
    vector<Token> t;

    ostringstream os;

    int i = 0;
    while (tests[i].input != "") {

        t = p->getPostfix(tests[i].input, 1);

        vector<Token>::iterator it;
        for (it = t.begin(); it != t.end(); it++) {
            os << it->word << " ";
        }

        if (os.str() != tests[i].output) {
            error = true;
            cout << "__ERROR______________________" << endl;
            cout << "  INPUT  |  " << tests[i].input << endl;
            cout << "  OUTPUT |  " << os.str() << endl;
            cout << "  EXPECT |  " << tests[i].output << endl;
        }

        os.str("");
        i++;
    }

    if (!error) {
        cout << "No errors in Postfix" << endl;
    }

    delete p;

}
