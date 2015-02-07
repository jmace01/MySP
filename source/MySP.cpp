#include <iostream>
#include "OperationNode.h"
#include "./Test/Test.h"
#include <vector>
#include "ExpressionTreeBuilder.h"

using namespace std;


int main() {

    string s = "2 + a[1]";

    ExpressionTreeBuilder* etb = new ExpressionTreeBuilder();

    try {
    	OperationNode* t = etb->getExpressionTree(s, 1);
    	cout << t->getTreePlot(0) << endl;
    	delete t;
    } catch (PostfixError &e) {
    	cout << e.msg << endl;
    }

    cout << endl << endl;
    delete etb;

    //Unit test
	Test();
}
