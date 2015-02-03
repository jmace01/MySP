#include <iostream>
#include "OperationNode.h"
#include "postfix.h"
#include "./Test/Test.h"
#include <vector>

using namespace std;


int main() {

    string s = "var += something*5.768+(.03+7) /*What?*/.\"This is a test\" //Hello!";
    s = "4++ + 3--";

    Postfix* p = new Postfix();
    try {
    	OperationNode* t = p->getPostfix(s, 1);
    	cout << t->getPostfix() << endl;
    } catch (PostfixError &e) {
    	cout << e.msg << endl;
    }

    cout << endl << endl;
    delete p;

    //Unit test
	Test();
}
