
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
void Test::testExecutor() {

    TestIO tests[] = {
            TestIO(
                    "main { print (1 < 2) . (2 > 1) . (1 == '1') . (1 === 1) . (a ==== a) . (1 != 2) . (1 !== '1') . (a !=== b) . (1 <= 2) . (2 >= 1) . (!0); }",
                    "11111111111"
            ),
            TestIO(
                    "main { print (1 > 2) . (2 < 1) . (1 != '1') . (1 !== 1) . (a !=== a) . (1 == 2) . (1 === '1') . (a ==== b) . (1 >= 2) . (2 <= 1) . (!1); }",
                    "00000000000"
            ),
            TestIO(
                    "main { print 10-1; print 6+3; print 2*4.5; print 3^2; print 18/2; print 19%10; print '9'.'9'; print 9++; print 9--; print -1 - -10; }",
                    "99999999999"
            ),
            TestIO(
                    "main { a = 10; a = a + 2; print a++; a = 'The answer is ' . a; print a; }",
                    "12The answer is 13"
            ),
            TestIO(
                    "main { whichCase = 1; if (whichCase == 1) { print \"YES!\"; } else if (whichCase == 2) { print \"NO!\"; } else { print \"?\"; } }",
                    "YES!"
            ),
            TestIO(
                    "main { whichCase = 2; if (whichCase == 1) { print \"YES!\"; } else if (whichCase == 2) { print \"NO!\"; } else { print \"?\"; } }",
                    "NO!"
            ),
            TestIO(
                    "main { whichCase = 3; if (whichCase == 1) { print \"YES!\"; } else if (whichCase == 2) { print \"NO!\"; } else { print \"?\"; } }",
                    "?"
            ),
            TestIO(
                    "main { x = 0; while (x < 10) { print x++; } }",
                    "0123456789"
            ),
            TestIO(
                    "main { for (i=0; i < 10; i++) { print i; } }",
                    "0123456789"
            ),
            TestIO(
                    "main { j = 0; do { print j++; } while (j < 10); }",
                    "0123456789"
            ),
            TestIO(
                    "main { v4 = v3 = v2 = v1 = 0; (1==1) && (v1 = 1); (1==2) && (v2 = 1); (1==1) || (v3 = 1); (1==2) || (v4 = 1); print v1.v2.v3.v4; }",
                    "1001"
            ),
            TestIO(
                    "main { a = 0 ? 'YES' : 'NO'; b = 1 ? 'YES' : 'NO'; print a . ', ' . b; }",
                    "NO, YES"
            ),
            TestIO(
                    "main { a = array(); a[0] = array(); a[0][0] = 1; print a . ', ' . a[0] . ', ' . a[0][0]; }",
                    "ARRAY, ARRAY, 1"
            ),
            TestIO(
                    "class demo { public dynamic a; public static e; } main { d = demo(); d->a = 1; demo::e = 2; print d->a; print demo::e; }",
                    "12"
            ),
            TestIO(
                    "class demo { private dynamic a; } main { d = demo(); d->a; }",
                    "ERROR: Cannot access private variable from outside class (line 1)\n"
            ),
            TestIO(
                    "class demo { private static a; } main { demo::a; }",
                    "ERROR: Cannot access private variable from outside class (line 1)\n"
            ),
            TestIO(
                    "main { bar->foo; }",
                    "ERROR: Cannot get property of undefined (line 1)\n"
            ),
            TestIO(
                    "main { random::foo; }",
                    "ERROR: Unknown class 'random' (line 1)\n"
            ),
            TestIO(
                    "main { foo++; }",
                    "WARNING: Cannot use ++ operator on type 'undefined' (line 1)\n"
            ),
            TestIO(
                    "main { 1/0; }",
                    "WARNING: Division by 0 (line 1)\n"
            ),
            TestIO(
                    "main { g[0]; }",
                    "WARNING: Cannot index from type undefined (line 1)\n"
            ),
            TestIO(
                    "main { 1 = 1; }",
                    "WARNING: Assignment to value instead of variable (line 1)\n"
            ),
            TestIO(
                    "main { true++; }",
                    "ERROR: Cannot increment a constant (line 1)\n"
            ),
            TestIO(
                    " class b {                      \n" \
                    "    private dynamic blah() {    \n" \
                    "       return 1;                \n" \
                    "    }                           \n" \
                    " }                              \n" \
                    " class a inherits b {           \n" \
                    "    public dynamic a() {        \n" \
                    "       print \"CONSTRUCTED\n\"; \n" \
                    "    }                           \n" \
                    "    public dynamic foo() {      \n" \
                    "       this->blah();            \n" \
                    "       return this->bar();      \n" \
                    "    }                           \n" \
                    "    private dynamic bar() {     \n" \
                    "       return 2;                \n" \
                    "    }                           \n" \
                    " }                              \n" \
                    " main {                         \n" \
                    "    print \"Hello world!\n\";   \n" \
                    "    v = a();                    \n" \
                    "    print v->foo() . \"\n\";    \n" \
                    "    print v->bar() . \"\n\";    \n" \
                    " }                              ",
                    "Hello world!\nCONSTRUCTED\nFATAL ERROR: Method 'blah' of class does not exist (line 11)\n2\nFATAL ERROR: Permission denied to call 'bar' (line 22)\n"
            ),
            TestIO(
                    "main { print (1 == 1) ? 1 : 2; }",
                    "1"
            ),
            TestIO(
                    "main { print ((1 == 1) ? 1 : 2) . 'Yes'; }",
                    "1Yes"
            ),
            TestIO(
                    "",
                    ""
            )
    };

    int i = 0;
    int passed = 0;
    bool error = false;

    cout << "Starting Executor Unit Test" << endl;
    clock_t timer = clock();

    //Redirect output
    stringstream   outputStream;
    streambuf*     oldbuf  = cout.rdbuf(outputStream.rdbuf());





    Parser sp = Parser();
    Executor ex = Executor();
    queue<PostfixError> pe;
    queue<Token> toks;

    while (tests[i].input != "") {
        outputStream.str("");
        toks = queue<Token>();
        try {
            Tokenizer().getTokens(tests[i].input, toks);
            map<string, ClassDefinition* >* ops = sp.parseTokens(toks);
            pe = sp.getErrors();
            if (pe.empty()) {
                ex.run(ops);
            } else {
                while (!pe.empty()) {
                    cout << pe.front().msg << " (line " << pe.front().t.line << ")" << endl;
                    pe.pop();
                }
            }
        } catch (PostfixError &e) {
            cout << e.msg << " (line " << e.t.line << ")" << endl;
        }

        //Tokenize statement
        if (outputStream.str() != tests[i].output) {
            error = true;
            cout.rdbuf(oldbuf);
            cout << "__ERROR______________________" << endl;
            cout << "  INPUT  |  " << tests[i].input << endl;
            cout << "  OUTPUT |  " << outputStream.str() << endl;
            cout << "  EXPECT |  " << tests[i].output << endl;
            oldbuf  = cout.rdbuf(outputStream.rdbuf());
        } else {
            passed++;
        }

        i++;
    }

    //Fix output
    cout.rdbuf(oldbuf);

    if (!error) {
        cout << "    No errors in ExpressionTreeBuilder unit test" << endl;
    }

    float targetSpeed = 0.0007 * i;
    float time = (float)(clock() - timer) / CLOCKS_PER_SEC;
    cout << "    Passed " << passed << "/" << i << " tests in " << time << " seconds" << endl;
    if (time > targetSpeed) cout << "    -- Processed too slow [should be " << targetSpeed << "]" << endl;
}
