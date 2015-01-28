#include <iostream>
#include "postfix.h"
#include "./Test/Test.h"
#include <vector>

using namespace std;


int main() {

    string s = "var += something*5.768+(.03+7) /*What?*/.\"This is a test\" //Hello!";
    s = "v = 3 == 4 ? (1 : 2)";

    Postfix* p = new Postfix();
    try {
    	vector<Token> t = p->getPostfix(s, 1);
    	vector<Token>::iterator it;
		for (it = t.begin(); it != t.end(); it++) {
			cout << it->word << " ";
		}
    } catch (PostfixError &e) {
    	cout << e.msg << endl;
    }

    cout << endl << endl;
    delete p;

    //Unit test
    try {
    	Test();
    } catch (PostfixError &e) {
    	cout << e.msg << endl;
    }
}
