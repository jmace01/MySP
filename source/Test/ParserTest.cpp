#include "Test.h"
#include "../token.h"
#include "../Tokenizer.h"
#include "../ClassDefinition.h"
#include "../Method.h"
#include <iostream>

using namespace std;

void Test::testParser() {

    TestIO tests[] = {
        TestIO( //Lone if statement unscoped
                "main { if (a == 2) print 'okay!'; }",
                "2 a == | 3 if | 'okay!' print"
        ),
        TestIO( //Lone if statement scoped
                "main { if (a == 2) { print 'okay!'; } }",
                "2 a == | 3 if | 'okay!' print"
        ),
        TestIO( //If/else statement scoped
                "main { if (x == 1) { x++; y++; } else { z++; } print x + y + z; }",
                "1 x == | 5 if | x ++ | y ++ | 6 :jmp | z ++ | z y + x + print"
        ),
        TestIO( //If/else statement unscoped
                "main { if (x == 1) x++; else y++; }",
                "1 x == | 4 if | x ++ | 5 :jmp | y ++"
        ),
        TestIO( //If/elseif/else statement unscoped
                "main { if (x == 1) true; else if (y == 2) false; else A; }",
                "1 x == | 4 if | true | 9 :jmp | 2 y == | 8 if | false | 9 :jmp | A"
        ),
        TestIO( //If/elseif/else statement scoped
                "main { if (x == 1) { true; } else if (y == 2) { false; } else { A; } }",
                "1 x == | 4 if | true | 9 :jmp | 2 y == | 8 if | false | 9 :jmp | A"
        ),
        TestIO( //Nested if unscoped
                "main { if (true) { if (false) true; else false; } }",
                "true | 7 if | false | 6 if | true | 7 :jmp | false"
        ),
        TestIO( //Nested if scoped
                "main { if (true) { if (false) { true; } else { false; } } }",
                "true | 7 if | false | 6 if | true | 7 :jmp | false"
        ),
        TestIO( //Nested if/else inside if/else
                "main { if (true) { if (false) true; else false; } else { return 1; } }",
                "true | 8 if | false | 6 if | true | 7 :jmp | false | 9 :jmp | 1 return"
        ),
        TestIO( //Invalid use of else with scope
                "main { if (true) { 1; } x++; else { 7; } }",
                "Unexpected 'else' not following 'if'"
        ),
        TestIO( //Invalid use of else without scope
                "main { if (true) 1; x++; else 7; }",
                "Unexpected 'else' not following 'if'"
        ),
        TestIO( //If without condition
                "main { if }",
                "Unexpected '}' after construct keyword | Expecting '}' to end method"
        ),
        TestIO( //While loop with scope
                "main { while (1) { print '1'; } }",
                "2 :jmp | '1' print | 1 | 5 if | 1 :jmp"
        ),
        TestIO( //While loop without scope
                "main { while (1) print '1'; }",
                "2 :jmp | '1' print | 1 | 5 if | 1 :jmp"
        ),
        TestIO( //If inside of while loop
                "main { while (1) { if (0) print '1'; } }",
                "4 :jmp | 0 | 4 if | '1' print | 1 | 7 if | 1 :jmp"
        ),
        TestIO( //Do while loop
                "main { do { B; } while (A); }",
                "B | A | 4 if | 0 :jmp"
        ),
        TestIO( //Do while loop missing while
                "main { do { B; } }",
                "Expecting 'while' after 'do'"
        ),
        TestIO( //Do while loop missing curly brace
                "main { do B; while (A); }",
                "Expecting '{' after 'do'"
        ),
        TestIO( //Do while loop missing ';'
                "main { do { B; } while (A) }",
                "Expecting ';' after do while loop"
        ),
        TestIO(
                "main { for (x; A; y) { B; } }",
                "x | 4 :jmp | B | y | A | 7 if | 2 :jmp"
        ),
        TestIO(
                "main { for (A) { B; }",
                "Expecting ';' in FOR loop condition | Unexpected '{' in statement | Unexpected Operator {"
        ),
        TestIO(
                "main { for (;;) b; }",
                "2 :jmp | b | 1 | 5 if | 1 :jmp"
        ),
        TestIO(
                "main { for (a;;;) { B; } }",
                "Unexpected Operator ;"
        ),
        TestIO(
                "main { for a { b; } }",
                "Expecting condition in FOR loop | Unexpected '{' in statement | Unexpected Operator { | Use of global statements is forbidden"
        ),
        TestIO(
                "c++",
                "Use of global statements is forbidden"
        ),
        TestIO(
                "main { try { print 1; } catch (Exception e) { print 2; } }",
                "6 2 try | 1 print | Exception 6 catch | e 6 exception | 2 print"
        ),
        TestIO(
                "main { try { print 1; } catch { print 2; } }",
                "Expected '(' after catch | Expecting '{' after 'catch'"
        ),
        TestIO(
                "main { try { print 1; } }",
                "Expected 'catch' after 'try'"
        ),
        TestIO(
                "main { catch { print 2; } }",
                "Unexpected 'catch' | Expected '(' after catch | Expecting '{' after 'catch'"
        ),
        TestIO(
                "main { throw 1; }",
                "1 throw"
        ),
        TestIO(
                "main { try { print 1; } catch (Exception e) { print 2; } finally { print 3; } }",
                "6 2 try | 1 print | Exception 6 catch | e 6 exception | 2 print | 8 finally | 3 print"
        ),
        TestIO(
                "main { finally { print 3; } }",
                "Unexpected 'finally'"
        ),
        TestIO(
                "main { try { print 1; } finally { print 3; } }",
                "Expected 'catch' after 'try' | Unexpected 'finally'"
        ),
        TestIO(
                "main { catch  (Exception e) { print 2; } finally { print 3; } }",
                "Unexpected 'catch'"
        ),
        TestIO(
                "main { try { throw 1; } catch (Exception1 e1) { print e1; } catch (Exception2 e2) { print e2; } finally { print 'done'; } }",
                "9 2 try | 1 throw | Exception1 6 catch | e1 9 exception | e1 print | Exception2 9 catch | e2 9 exception | e2 print | 11 finally | 'done' print"
        ),
        TestIO( //missing semicolon in try
                "main { try { throw 1 } catch (Exception e) { print e; } }",
                "Unexpected '}' in statement"
        ),
        TestIO(
                "main { try {} catch (Exception e) {} }",
                "4 1 try | Exception 4 catch | e 4 exception"
        ),
        TestIO(
                "main { try {} catch (1 + 1) {} }",
                "Expected exception class type | Expecting '{' after 'catch' | Expecting ';' in statement | Unexpected '{' in statement | Unexpected '}' in statement"
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

    map<string, ClassDefinition* >* ops;
    map<string, ClassDefinition* >::iterator it;
    map<string, Method*> methods;
    map<string, Method*>::iterator it2;
    Tokenizer tokenizer;

    cout << "Starting Parser Unit Test" << endl;
    clock_t timer = clock();

    while (tests[i].input != "") {
        output = "";
        errorState = false;

        try {
            toks = queue<Token>();
            tokenizer.getTokens(tests[i].input, toks);
            ops = sp.parseTokens(toks);
            pe = sp.getErrors();
            while (!pe.empty()) {
                if (output != "") output += " | ";
                output += pe.front().msg;
                pe.pop();
            }
            sp.clearErrors();
            errorState = (output != "");
            if (ops != NULL) {
                for (it = (*ops).begin(); it != (*ops).end(); it++) {
                    methods = it->second->getMethods();
                    for (it2 = methods.begin(); it2 != methods.end(); it2++) {
                        for (int ii = 0; ii < it2->second->getInstructionTreeSize(); ii++) {
                            if (!errorState) {
                                if (ii != 0) output += " | ";
                                output += it2->second->getInstruction(ii)->getPostfix();
                            }
                        }
                    }
                    delete it->second;
                }
                delete ops;
            }
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
