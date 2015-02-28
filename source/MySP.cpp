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

    string s = "function main() { print \"Testing\"; }";

    Parser sp = Parser();
    queue<PostfixError> pe;

    try {
        map<string, vector<OperationNode*> >* ops = sp.parseText(s);
        map<string, vector<OperationNode*> >::iterator it;
        pe = sp.getErrors();
        while (!pe.empty()) {
            cout << pe.front().msg << endl;
            pe.pop();
        }
        for (it = (*ops).begin(); it != (*ops).end(); it++) {
            for (int i = 0; i < (*ops)[it->first].size(); i++) {
                cout << "------------ { " << it->first << " : " << i << " } ------------" << endl;
                cout << (*ops)[it->first].at(i)->getTreePlot(0);
            }
            while (!(*ops)[it->first].empty()) {
                delete (*ops)[it->first].back();
                (*ops)[it->first].pop_back();
            }
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
