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


// For testing //
#include "ExpressionTreeBuilder.h"
#include "ExpressionTreeFlattener.h"
#include "OperationNode.h"
#include <vector>
#include <iomanip>
// For testing //


using namespace std;


int main(int argc, char ** argv) {

    //////////////////////////////////
    //////////////////////////////////
    //////////////////////////////////
    queue<Token> tks = queue<Token>();

    //string sin = "a() && b()";
    //string sin = "1 && 1";
    string sin = "a = b ? 1 : 1";

    Tokenizer t;
    ExpressionTreeBuilder eb;
    ExpressionTreeFlattener ef;
    OperationNode* root;
    vector<Instruction> instructions;

    try {
        t.getTokens(sin, tks);
        root = eb.getExpressionTree(tks);
        ef.flattenTree(root, instructions, 0);

        int i=0;
        vector<Instruction>::iterator it;
        cout << "+-----+------------+-------------+-------------+------+------+" << endl;
        cout << "|  #  | INST CODE  | OP A        | OP B        | TYPE | TYPE |" << endl;
        cout << "+-----+------------+-------------+-------------+------+------+" << endl;
        for (it = instructions.begin(); it != instructions.end(); it++) {
            cout << "| "
                 << setw(4)
                 << left
                 << i++
                 << "| "
                 << setw(11)
                 << left
                 << it->instruction << "| ";
            if (it->aType == 'n') cout << setw(12) << left << it->operandAd;
            else cout << setw(12) << left << (it->aType == 'r' ? "<REG>" : it->operandAs);
            cout << "| ";
            if (it->bType == 'n') cout << setw(12) << left << it->operandBd;
            else cout << setw(12) << left << (it->bType == 'r' ? "<REG>" : it->operandBs);
            cout << "| " << it->aType << "    |"
                 << " " << it->bType << "    |" << endl;
        }
        //cout << "| " << setw(4) << left << i << "|" << " 0          | -           | -           | -    | -    |" << endl;
        cout << "+-----+------------+-------------+-------------+------+------+" << endl;
        cout << "|  #  | INST CODE  | OP A        | OP B        | TYPE | TYPE |" << endl;
        cout << "+-----+------------+-------------+-------------+------+------+" << endl;
    } catch (PostfixError &e) {
        cout << e.msg << endl;
    }
    return 0;
    //////////////////////////////////
    //////////////////////////////////
    //////////////////////////////////



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
