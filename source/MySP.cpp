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

    string s = "if (true) { if (false) true; else false; }";

    Parser sp = Parser();

    try {
        map<string, vector<OperationNode*> >* ops = sp.parseText(s);
        for (int i = 0; i < (*ops)["~"].size(); i++) {
            cout << "---------- { " << i << " } ----------" << endl;
            cout << (*ops)["~"].at(i)->getTreePlot(0);
        }
        cout << "---------------------------" << endl;
    } catch (PostfixError &e) {
        cout << e.msg << endl;
    }

    /*
    string s = "continue";

    ExpressionTreeBuilder* etb = new ExpressionTreeBuilder();

    try {
    	OperationNode* t = etb->getExpressionTree(s, 1);
    	if (t != NULL) {
            cout << t->getTreePlot(0) << endl;
            delete t;
    	}
    } catch (PostfixError &e) {
    	cout << e.msg << endl;
    }

    cout << endl << endl;
    delete etb;
    */

    //Unit test
	Test();
}
