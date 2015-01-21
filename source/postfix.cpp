#include "postfix.h"
#include <iostream>
#include <ctype.h>
#include <queue>

using namespace std;


/******************************************************************
 *
 ******************************************************************/
Postfix::Postfix() {
    this->infix      = "";
    this->infixPos   = 0;
    this->lineNumber = 0;
}


/******************************************************************
 *
 ******************************************************************/
Postfix::~Postfix() {
}


/******************************************************************
 *
 ******************************************************************/
vector<Token> Postfix::getPostfix(string infix, unsigned int lineNumber) {
    this->infix            = infix;
    this->infixPos         = 0;
    this->lineNumber       = lineNumber;
    stack<Token> operands  = stack<Token>();
    stack<Token> operators = stack<Token>();

    int parenths           = 0;     //Parenthesis that are still open
    bool expectingOperator = false; //Is it time for an operator?
    bool wasWord           = false; //Was the last a word? Needed for function calls

    queue<Token> toks = this->getTokens();
    vector<Token> result = vector<Token>();

    Token p; //Temporary for holding tokens

    Token t;
    while (!toks.empty()) {
        t = toks.front();
        toks.pop();

        //Pre Unary operators, such as ! or ~
        if (t.type == 'o' && this->isPreUnary(t.word)) {
            operators.push(t);

        //Post Unary Operators, such as ++ or --
        } else if (t.type == 'o' && this->isPostUnary(t.word)) {
            if (operands.size() == 0) {
                cout << "ERROR not enough operands " << t.word << endl;
                continue;
            }

            p = operands.top();
            operands.pop();
            result.push_back(p);

            result.push_back(t);

            p = Token();
            p.line = -1;
            p.type = 'w';
            p.word = "TEMP";
            operands.push(p);

        //Normal operators
        } else if (t.type == 'o') {
            while (t.word != "(" && t.word != "[" && operators.size() != 0 && this->getOperatorHeirchy(operators.top().word) > 0 && !this->isPreUnary(operators.top().word) && this->getOperatorHeirchy(t.word) < this->getOperatorHeirchy(operators.top().word)) {
                if (operands.size() < 2) {
                    cout << "ERROR not enough operands " << t.word << endl;
                    break;
                }
                p = operands.top();
                operands.pop();
                result.push_back(p);
                p = operands.top();
                operands.pop();
                result.push_back(p);

                p = operators.top();
                operators.pop();
                result.push_back(p);

                p = Token();
                p.line = -1;
                p.type = 'w';
                p.word = "TEMP";
                operands.push(p);
            }
            if (t.word == ")" && operators.size() > 0 && operators.top().word == "(") {
                operators.pop();
            }
            if (t.word != "(" && t.word != "[" && operators.size() > 0 && this->isPreUnary(operators.top().word)) {
                p = operands.top();
                operands.pop();
                result.push_back(p);

                p = operators.top();
                operators.pop();
                result.push_back(p);

                p = Token();
                p.line = -1;
                p.type = 'w';
                p.word = "TEMP";
                operands.push(p);
            }
            if (t.word != ")" && t.word != "]") {
                operators.push(t);
            }

        //Operands
        } else {
            operands.push(t);
        }
    }

    while (operators.size() > 0 && operands.size() >= 2) {
        if (!this->isPreUnary(operators.top().word)) {
            p = operands.top();
            operands.pop();
            result.push_back(p);
        }
        p = operands.top();
        operands.pop();
        result.push_back(p);

        p = operators.top();
        operators.pop();
        result.push_back(p);

        p = Token();
        p.line = -1;
        p.type = 'w';
        p.word = "TEMP";
        operands.push(p);
    }

    while (operands.size() > 0) {
        p = operands.top();
        operands.pop();
        result.push_back(p);
    }

    return result;
}


/******************************************************************
 *
 ******************************************************************/
queue<Token> Postfix::getTokens() {
    queue<Token> result = queue<Token>();

    string s;
    Token t;
    while (this->infixPos < this->infix.size()) {
        t = this->getNext();
        if (t.word != "") {
            result.push(t);
        }
    }

    return result;
}


/******************************************************************
 *
 ******************************************************************/
Token Postfix::getNext() {
    //Initialize variable
    string word = "";
    char type;

    //Use "i" instead of "this->infixPos"
    int i = this->infixPos;


    //Eat whitespace
    while (
            this->infix[i] == ' '  ||
            this->infix[i] == '\r' ||
            this->infix[i] == '\n' ||
            this->infix[i] == '\t'
          ) {
        if (this->infix[i] == '\n') {
            this->lineNumber++;
        }
        i++;
    }


    //Eat comments
    if (this->infix[i] == '/' && (this->infix[i + 1] == '/' || this->infix[i+1] == '*')) {
        i++;
        if (this->infix[i] == '/') { // for "//" comments
            i++;
            while (this->infix[i] != '\n' && this->infix[i] != '\r' && i < this->infix.size()) {
                i++;
            }
            this->lineNumber++;
        } else { //for "/**/" comments
            i++;
            while (this->infix[i] != '*' && this->infix[i+1] != '/' && i < this->infix.size()) {
                if (this->infix[i] == '\n' || this->infix[i] == '\r') {
                    this->lineNumber++;
                }
                i++;
            }
            i += 2;
        }
    }


    //Save the line number for the token
    int line = this->lineNumber;


    //Get String
    if (this->infix[i] == '"' || this->infix[i] == '\'')
    {
        type = 's';
        char delimiter = this->infix[i];
        bool slash = false;
        do
        {
            slash = (!slash && this->infix[i] == '\\');
            word += this->infix[i++];
        }
        while ((this->infix[i] != delimiter || slash) && i < this->infix.size());
        word += this->infix[i++];
    }


    //Get number
    else if (isnumber(this->infix[i]) //its a number
            || (    //its a decimal followed by a number
                    this->infix[i] == '.' &&
                    i < this->infix.size() - 1 && //don't walk off the array
                    isnumber(this->infix[i + 1])
                )
            )
    {
        type = 'n';
        bool dec = false; //Was a decimal used yet?
        while (isnumber(this->infix[i])
              || (
                      !dec && this->infix[i] >= '.' &&
                      i < this->infix.size() - 1    &&
                      isnumber(this->infix[i + 1])
                  )
              ) {
            if (this->infix[i] == '.') {
                dec = true;
            }
            word += this->infix[i++];
        }
    }


    //Get Word
    else if (isalpha(this->infix[i])) {
        type = 'w';
        while (isalnum(this->infix[i]) &&  i <  this->infix.size())
            word += this->infix[i++];
    }


    //Get Operator
    else {
        type = 'o';
        while (!isalnum(this->infix[i]) && i < this->infix.size()) {
            word += this->infix[i++];
            if (getOperatorHeirchy(word + this->infix[i]) == 0) {
                break;
            }
        }
        if (word != "" && this->getOperatorHeirchy(word) == 0) {
            cout << "ERROR Unknown operator " << word << endl;
        }
    }


    //Save position
    this->infixPos = i;

    //Create the return token
    Token t = Token();
    t.word = word;
    t.line = line;
    t.type = type;

    //Return the token
    return t;
}


/******************************************************************
 *
 ******************************************************************/
int Postfix::getOperatorHeirchy(std::string op) {
    if (    op == "print"  ||
            op == "echo"   ||
            op == "return" ||
            op == "break"  ||
            op == "continue"
        )
        return 1;

    if (    op == "="  ||
            op == "+=" ||
            op == "-=" ||
            op == "*=" ||
            op == "/=" ||
            op == "%=" ||
            op == "^=" ||
            op == "as"
        )
        return 2;

    if (    op == "====" ||
            op == "==="  ||
            op == "=="   ||
            op == "!===" ||
            op == "!=="  ||
            op == "!="   ||
            op == "<"    ||
            op == "<="   ||
            op == ">"    ||
            op == ">="   ||
            op == "=>"
        )
        return 3;

    if (    op == "&&"  ||
            op == "||"  ||
            op == "|||"
        )
        return 4;

    if (    op == "."
        )
        return 5;

    if (    op == "-" ||
            op == "+"
        )
        return 6;

    if (    op == "*" ||
            op == "/" ||
            op == "%"
        )
        return 7;

    if (
            op == "^"
        )
        return 8;

    //Postfixed Unary Operators
    if (    op == "++" ||
            op == "--"
        )
        return 9;

    //Prefixed Unary Operators
    if (    op == "~" ||
            op == "!"
        )
        return 10;

    if (    op == "(" ||
            op == ")" ||
            op == "[" ||
            op == "]"
        )
        return -1;

    return 0;
}


/******************************************************************
 *
 ******************************************************************/
bool Postfix::isPostUnary(string op) {
    int h = this->getOperatorHeirchy(op);
    return (h == 9);
}


/******************************************************************
 *
 ******************************************************************/
bool Postfix::isPreUnary(string op) {
    int h = this->getOperatorHeirchy(op);
    return (h == 10);
}
