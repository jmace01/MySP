#include "Test.h"
#include <iostream>

using namespace std;

void Test::testParser() {

    TestIO tests[] = {
        TestIO( //Lone if statement unscoped
                "function main() { if (a == 2) print 'okay!'; }",
                "2 a == | 3 if | 'okay!' print"
        ),
        TestIO( //Lone if statement scoped
                "function main() { if (a == 2) { print 'okay!'; } }",
                "2 a == | 3 if | 'okay!' print"
        ),
        TestIO( //If/else statement scoped
                "function main() { if (x == 1) { x++; y++; } else { z++; } print x + y + z; }",
                "1 x == | 5 if | x ++ | y ++ | 6 jmp | z ++ | z y + x + print"
        ),
        TestIO( //If/else statement unscoped
                "function main() { if (x == 1) x++; else y++; }",
                "1 x == | 4 if | x ++ | 5 jmp | y ++"
        ),
        TestIO( //If/elseif/else statement unscoped
                "function main() { if (x == 1) true; else if (y == 2) false; else A; }",
                "1 x == | 4 if | true | 9 jmp | 2 y == | 8 if | false | 9 jmp | A"
        ),
        TestIO( //If/elseif/else statement scoped
                "function main() { if (x == 1) { true; } else if (y == 2) { false; } else { A; } }",
                "1 x == | 4 if | true | 9 jmp | 2 y == | 8 if | false | 9 jmp | A"
        ),
        TestIO( //Nested if unscoped
                "function main() { if (true) { if (false) true; else false; } }",
                "true | 7 if | false | 6 if | true | 7 jmp | false"
        ),
        TestIO( //Nested if scoped
                "function main() { if (true) { if (false) { true; } else { false; } } }",
                "true | 7 if | false | 6 if | true | 7 jmp | false"
        ),
        TestIO( //Nested if/else inside if/else
                "function main() { if (true) { if (false) true; else false; } else { return 1; } }",
                "true | 8 if | false | 6 if | true | 7 jmp | false | 9 jmp | 1 return"
        ),
        TestIO( //Invalid use of else with scope
                "function main() { if (true) { 1; } x++; else { 7; } }",
                "Unexpected 'else' not following 'if'"
        ),
        TestIO( //Invalid use of else without scope
                "function main() { if (true) 1; x++; else 7; }",
                "Unexpected 'else' not following 'if'"
        ),
        TestIO( //If without condition
                "function main() { if }",
                "Unexpected '}' after construct keyword | Unexpected '}'"
        ),
        TestIO( //While loop with scope
                "function main() { while (1) { print '1'; } }",
                "2 jmp | '1' print | 1 | 5 if | 1 jmp"
        ),
        TestIO( //While loop without scope
                "function main() { while (1) print '1'; }",
                "2 jmp | '1' print | 1 | 5 if | 1 jmp"
        ),
        TestIO( //If inside of while loop
                "function main() { while (1) { if (0) print '1'; } }",
                "4 jmp | 0 | 4 if | '1' print | 1 | 7 if | 1 jmp"
        ),
        TestIO( //Do while loop
                "function main() { do { B; } while (A);",
                "B | A | 4 if | 0 jmp"
        ),
        TestIO( //Do while loop missing while
                "function main() { do { B; } }",
                "Expecting 'while' after 'do'"
        ),
        TestIO( //Do while loop missing curly brace
                "function main() { do B; while (A); }",
                "Expecting '{' after 'do'"
        ),
        TestIO( //Do while loop missing ';'
                "function main() { do { B; } while (A) }",
                "Expecting ';' after do while loop"
        ),
        TestIO(
                "function main() { for (x; A; y) { B; } }",
                "x | 4 jmp | B | y | A | 7 if | 2 jmp"
        ),
        TestIO(
                "function main() { for (A) { B; }",
                "Expecting ';' in FOR loop condition | Unexpected closing parenthesis"
        ),
        TestIO(
                "function main() { for (;;) b; }",
                "2 jmp | b | 1 | 5 if | 1 jmp"
        ),
        TestIO(
                "function main() { for (a;;;) { B; } }",
                "Unexpected Operator ;"
        ),
        TestIO(
                "function main() { for a { b; } }",
                "Expecting condition in FOR loop | Use of global statements is forbidden | Unexpected '}'"
        ),
        TestIO(
                "c++",
                "Use of global statements is forbidden"
        ),
        TestIO(
                "",
                ""
        ),
        TestIO(
                "",
                ""
        ),
        TestIO(
                "",
                ""
        ),
        TestIO(
                "",
                ""
        ),
        TestIO(
                "",
                ""
        )
    };



    Parser sp = Parser();
    queue<PostfixError> pe;

    queue<Token> toks;
    string output;

    int i = 0;
    int passed = 0;
    bool error = false;
    bool errorState = false;

    map<string, vector<OperationNode*> >* ops;
    map<string, vector<OperationNode*> >::iterator it;

    cout << "Starting Parser Unit Test" << endl;
    clock_t timer = clock();

    while (tests[i].input != "") {
        output = "";
        errorState = false;

        try {
            ops = sp.parseText(tests[i].input);
            pe = sp.getErrors();
            while (!pe.empty()) {
                if (output != "") output += " | ";
                output += pe.front().msg;
                pe.pop();
            }
            sp.clearErrors();
            errorState = (output != "");
            for (it = (*ops).begin(); it != (*ops).end(); it++) {
                for (int ii = 0; ii < (*ops)[it->first].size(); ii++) {
                    if (!errorState) {
                        if (ii != 0) output += " | ";
                        output += (*ops)[it->first].at(ii)->getPostfix();
                    }
                }
                while (!(*ops)[it->first].empty()) {
                    delete (*ops)[it->first].back();
                    (*ops)[it->first].pop_back();
                }
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
