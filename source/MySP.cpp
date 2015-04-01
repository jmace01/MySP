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


int main(int argc, char ** argv) {

    //Get the file to run
    istream* s;
    //Was a file fiven?
    if (argc < 2) {
        cout << "You must specify a filename" << endl;
        return 1;
    }
    //Can the file be run?
    else {
        s = new ifstream(argv[1]);
        if (!s->good()) {
            cout << "The file cannot be run!" << endl;
            return 1;
        }
    }

    //For testing purposes
    //----------------------------------------------
    //string s = "";
    //istringstream* s = new istringstream("");
    //istream* s = new ifstream("./final.mysp");
    //----------------------------------------------

    //Do we want to display the time it took to process?
    bool timed = (argc > 2);
    //Do we want to execute unit tests?
    bool test  = (argc > 3);

    //Start time
    clock_t timer = clock();

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
        cout << endl << "RAN IN : ";
        cout << ((float)(clock() - timer) / CLOCKS_PER_SEC) << endl;
    }

    //Unit test
	if(test) {
	    Test();
	}

	//Ensure all data was output to the console
	cout.flush();

	return 0;
}
