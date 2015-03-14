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

    //string s = "main { (a = 1) && (b = 1); }";
    //istringstream* s = new istringstream("main { print 5; }");
    istream* s = new ifstream("./test.mysp");

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
        cout << e.msg << endl;
    }

    cout << endl << endl;

    //Unit test
	Test();
}
