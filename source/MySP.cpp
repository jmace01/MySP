/*/////////////////////////////////////// !! ////////////////////////////////////////////
 *
 * FILE:
 *     MySP.cpp
 *
 * DESCRIPTION:
 *     The main execution file
 *
 * AUTHOR:
 *     Jason Mace
 *
 *
 * Copyright 2015 by Jason Mace
 *
 */////////////////////////////////////// !! ////////////////////////////////////////////


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

    //Do we want to display the time it took to process?
    bool timed = true;
    //Do we want to execute unit tests?
    bool test  = true;

    //Start time
    clock_t timer = clock();

    //string s = "";
    //istringstream* s = new istringstream("");
    istream* s = new ifstream("./final.mysp");

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

    //End time
    if (timed) {
        cout << endl << endl;
        cout << ((float)(clock() - timer) / CLOCKS_PER_SEC) << endl;
    }

    //Unit test
	if(test) {
	    Test();
	}
}
