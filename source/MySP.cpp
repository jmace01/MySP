#include <iostream>
#include "OperationNode.h"
#include "./Test/Test.h"
#include <vector>
#include <string>
#include <map>
#include "ExpressionTreeBuilder.h"
#include "Parser.h"

using namespace std;


int main() {

    string s = "main { a++; }";

    Parser sp = Parser();
    queue<PostfixError> pe;

    try {
        map<string, ClassDefinition* >* ops = sp.parseText(s);
        map<string, ClassDefinition* >::iterator it;
        map<string, Method*> methods;
        map<string, Method*>::iterator it2;
        pe = sp.getErrors();
        while (!pe.empty()) {
            cout << pe.front().msg << endl;
            pe.pop();
        }
        for (it = (*ops).begin(); it != (*ops).end(); it++) {
            cout << "============ {[ Class " << it->first << " ]} ============" << endl;
            methods = it->second->getMethods();
            for (it2 = methods.begin(); it2 != methods.end(); it2++) {
                for (int i = 0; i < it2->second->getInstructionSize(); i++) {
                    cout << "------------ { " << it2->first << " : " << i << " } ------------" << endl;
                    cout << it2->second->getInstruction(i)->getTreePlot(0) << endl;
                }
            }
            delete it->second;
        }
        delete ops;
        cout << "--------------------------------------" << endl;
    } catch (PostfixError &e) {
        cout << e.msg << endl;
    }

    cout << endl << endl;

    //Unit test
	Test();
}
