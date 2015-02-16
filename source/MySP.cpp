#include <iostream>
#include "OperationNode.h"
#include "./Test/Test.h"
#include <vector>
#include "ExpressionTreeBuilder.h"
#include "StructureParser.h"

using namespace std;


int main() {

    StructureParser sp = StructureParser();

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
    /**/

    //Unit test
	Test();
}
