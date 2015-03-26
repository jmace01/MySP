#include <iostream>
#include <map>
#include "Executor.h"
#include "Parser.h"
#include "./Test/Test.h"
#include "Tokenizer.h"
#include <sstream>
#include <fstream>


using namespace std;


int main() {

    clock_t timer = clock();

    //string s = "main { a = 0 ? '1' : '2'; print a; }";
    //istringstream* s = new istringstream("main { print 5; }");
    istream* s = new ifstream("./test2.mysp");

    Parser sp = Parser();
    Executor ex = Executor();
    queue<PostfixError> pe;
    queue<Token> toks = queue<Token>();

    try {
        Tokenizer().getTokens(s, toks);
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

    cout << endl << endl;
    cout << ((float)(clock() - timer) / CLOCKS_PER_SEC) << endl;

    //Unit test
	Test();
}
