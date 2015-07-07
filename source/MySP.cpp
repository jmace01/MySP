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
#include "Options.h"


// For testing //
#include "ExpressionTreeBuilder.h"
#include "ExpressionTreeFlattener.h"
#include "OperationNode.h"
#include <vector>
#include <iomanip>
// For testing //


using namespace std;


int main(int argc, char ** argv) {

    Options options;
    options.parseOptions(argc, argv);

    //////////////////////////////////
    //////////////////////////////////
    //////////////////////////////////
    queue<Token> tks = queue<Token>();

    //string sin = "a() && b()";
    //string sin = "1 && 1";
    //string sin = "a = b ? 1 : 1";
    //string sin = "a::b->c->d + a::b";
    //string sin = "main { if (true) { if (false) { print 'YES'; } else { return; } } else { print 'NO!'; } }";
    //string sin = "main {  a = &b->c; }";
    string sin = "main { try { throw 7; } catch { print 2; } finally { print 3; } }";

    Tokenizer t;
    ExpressionTreeBuilder eb;
    ExpressionTreeFlattener ef;
    Parser par = Parser();
    queue<PostfixError> postE;

    clock_t timered = clock();

    try {
        t.getTokens(sin, tks);
        map<string, ClassDefinition* >* clss = par.parseTokens(tks);

        //root = eb.getExpressionTree(tks);
        //ef.flattenTree(root, instructions, 0);
        string s = "main";
        ef.flattenClass(*(*clss)["~"]);

        //Get any parse errors
        postE = par.getErrors();
        while (!postE.empty()) {
            cout << postE.front().msg << " (line " << postE.front().t.line << ")" << endl;
            postE.pop();
        }

        int i=0;
        vector<Instruction>::iterator it;
        vector<Instruction> instructions = (*clss)["~"]->getMethod(s)->getInstructionCodeVector();
        cout << sin << endl;
        cout << "+-----+---------------+-------------+-------------+------+------+" << endl;
        cout << "|  #  | INST CODE     | OP A        | OP B        | TYPE | TYPE |" << endl;
        cout << "+-----+---------------+-------------+-------------+------+------+" << endl;
        for (it = instructions.begin(); it != instructions.end(); it++) {
            cout << "| "
                 << setw(4)
                 << left
                 << i++
                 << "| "
                 << setw(3)
                 << left
                 << it->instruction << '[' << left << setw(10) << ExpressionTreeFlattener::lookupCode(it->instruction) + "]" << "|";
            if (it->aType == 'n' || it->aType == 'h') cout << ((it->aType == 'h') ? " #" : " ") << setw(11 + (it->aType != 'h')) << left << it->operandAd;
            else cout << ' ' << setw(12) << left << (it->aType == 'r' ? "<REG>" : it->operandAs);
            cout << "|";
            if (it->bType == 'n' || it->bType == 'h') cout << ((it->bType == 'h') ? " #" : " ") << setw(11 + (it->bType != 'h')) << left << it->operandBd;
            else cout << ' ' << setw(12) << left << (it->bType == 'r' ? "<REG>" : it->operandBs);
            cout << "| " << it->aType << "    |"
                 << " " << it->bType << "    |" << endl;
        }
        //cout << "| " << setw(4) << left << i << "|" << " 0          | -           | -           | -    | -    |" << endl;
        cout << "+-----+---------------+-------------+-------------+------+------+" << endl;
        cout << "|  #  | INST CODE     | OP A        | OP B        | TYPE | TYPE |" << endl;
        cout << "+-----+---------------+-------------+-------------+------+------+" << endl;
    } catch (PostfixError &e) {
        cout << e.msg << endl;
    }
    cout << endl << "RAN IN : ";
    cout << ((float)(clock() - timered) / CLOCKS_PER_SEC) << endl;
    Test();
    cout << endl << "RAN IN : ";
    cout << ((float)(clock() - timered) / CLOCKS_PER_SEC) << endl;

    //Executor test code goes here

    return 0;
    //////////////////////////////////
    //////////////////////////////////
    //////////////////////////////////



    //Get the file to run
    istream* s;
    //Was a file given?
    if (options.getFilename() == "") {
        cout << "You must specify a filename" << endl;
        return 1;
    }
    //Can the file be run?
    else {
        s = new ifstream(options.getFilename());
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
    bool timed = (options.getTimeExecution());
    //Do we want to execute unit tests?
    bool test  = (options.getRunUnitTests());

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
	    cout << "FINISHED" << endl;
	}

	//Ensure all data was output to the console
	cout.flush();

	return 0;
}
