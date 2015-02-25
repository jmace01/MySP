#include "Test.h"
#include <iostream>

using namespace std;

void Test::testParser() {

    TestIO tests[] = {
        TestIO( //Lone if statement unscoped
                "if (a == 2) print 'okay!';",
                "2 a == | 3 if | 'okay!' print"
        ),
        TestIO( //Lone if statement scoped
                "if (a == 2) { print 'okay!'; }",
                "2 a == | 3 if | 'okay!' print"
        ),
        TestIO( //If/else statement scoped
                "if (x == 1) { x++; y++; } else { z++; } print x + y + z;",
                "1 x == | 5 if | x ++ | y ++ | 6 jmp | z ++ | z y + x + print"
        ),
        TestIO( //If/else statement unscoped
                "if (x == 1) x++; else y++;",
                "1 x == | 4 if | x ++ | 5 jmp | y ++"
        ),
        TestIO( //If/elseif/else statement unscoped
                "if (x == 1) true; else if (y == 2) false; else A;",
                "1 x == | 4 if | true | 9 jmp | 2 y == | 8 if | false | 9 jmp | A"
        ),
        TestIO( //If/elseif/else statement scoped
                "if (x == 1) { true; } else if (y == 2) { false; } else { A; }",
                "1 x == | 4 if | true | 9 jmp | 2 y == | 8 if | false | 9 jmp | A"
        ),
        TestIO( //Nested if unscoped
                "if (true) { if (false) true; else false; }",
                "true | 7 if | false | 6 if | true | 7 jmp | false"
        ),
        TestIO( //Nested if scoped
                "if (true) { if (false) { true; } else { false; } }",
                "true | 7 if | false | 6 if | true | 7 jmp | false"
        ),
        TestIO( //Nested if/else inside if/else
                "if (true) { if (false) true; else false; } else { return 1; }",
                "true | 8 if | false | 6 if | true | 7 jmp | false | 9 jmp | 1 return"
        ),
        TestIO( //Invalid use of else with scope
                "if (true) { 1; } x++; else { 7; }",
                "Unexpected 'else' not following 'if'"
        ),
        TestIO( //Invalid use of else without scope
                "if (true) 1; x++; else 7;",
                "Unexpected 'else' not following 'if'"
        ),
        TestIO( //If without condition
                "if",
                "Unexpected end of file after construct keyword"
        ),
        TestIO(
                "",
                ""
        )
    };



    Parser sp = Parser();


    queue<Token> toks;
    string output;

    int i = 0;
    int passed = 0;
    bool error = false;

    map<string, vector<OperationNode*> >* ops;

    cout << "Starting Parser Unit Test" << endl;
    clock_t timer = clock();

    while (tests[i].input != "") {
        output = "";

        try {
            ops = sp.parseText(tests[i].input);
            for (int ii = 0; ii < (*ops)["~"].size(); ii++) {
                if (ii != 0) output += " | ";
                output += (*ops)["~"].at(ii)->getPostfix();
                delete (*ops)["~"].at(ii);
            }
            while (!(*ops)["~"].empty()) {
                (*ops)["~"].pop_back();
            }
            delete ops;
        } catch (PostfixError &e) {
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
        cout << "    No errors in Parser unit test" << endl;
    }

    float targetSpeed = 0.0003 * i;
    float time = (float)(clock() - timer) / CLOCKS_PER_SEC;
    cout << "    Passed " << passed << "/" << i << " tests in " << time << " seconds" << endl;
    if (time > targetSpeed) cout << "    -- Processed too slow [should be " << targetSpeed << "]" << endl;


}
