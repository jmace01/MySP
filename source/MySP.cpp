#include <iostream>
#include "postfix.h"
#include "./Test/Test.h"
#include <vector>

using namespace std;


int main() {

    string s = "var += something*5.768+(.03+7) /*What?*/.\"This is a test\" //Hello!";

    Postfix* p = new Postfix();
    vector<Token> t = p->getPostfix(s, 1);
    vector<Token>::iterator it;
    for (it = t.begin(); it != t.end(); it++) {
        cout << it->word << " ";
    }
    cout << endl << endl;
    delete p;


    //Unit test
    Test();
}
