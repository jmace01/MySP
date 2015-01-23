#include <iostream>
#include "postfix.h"
#include <vector>

using namespace std;


int main() {

    Postfix* p = new Postfix();
    //vector<Token> t = p->getPostfix("var += something*5.768+(.03+7) /*What?*/.\"This is a test\" //Hello!", 1);
    //vector<Token> t = p->getPostfix("4++ + 3--", 1);
    //vector<Token> t = p->getPostfix("!1 || !(1 * 0) && ~k", 1);
    //vector<Token> t = p->getPostfix("print \"Hello \" . \"world!\"", 1);
    //vector<Token> t = p->getPostfix("1", 1);
    vector<Token> t = p->getPostfix("v = var[x++]", 1);

    vector<Token>::iterator it;
    for (it = t.begin(); it != t.end(); it++) {
        cout << it->word << " ";
    }

    cout << endl;

    delete p;

}
