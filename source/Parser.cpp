#include "Parser.h"
#include <iostream>

using namespace std;

// #yolo -- turn off all error messages and warnings
// #selfie -- turn off wornings
// #whereisthelove -- set precision

/****************************************************************************************
 *
 ****************************************************************************************/
Parser::Parser() {
    this->expTreeBuilder = ExpressionTreeBuilder();
    this->functions = map< string, vector<OperationNode*> >();
    this->errors = queue<PostfixError>();
    this->currentFunction = "~";
    if (Parser::keywords.empty()) {
        initKeywords();
    }
}

//Initialize keywords map
map<string, short> Parser::keywords = map<string, short>();


/****************************************************************************************
 *
 ****************************************************************************************/
Parser::~Parser() {
    //
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Parser::parseText(string infix) {
    queue<Token> toks = queue<Token>();
    this->getTokens(infix, toks);

    Token t;
    string uppercaseWord;
    OperationNode* op;
    queue<Token> statementQueue;

    while (!toks.empty()) {
        //Get token off queue
        t = toks.front();
        toks.pop();

        //Don't allow leading or repeated semicolons
        if (t.word == ";") continue;

        //Convert to uppercase
        uppercaseWord = "";
        for (int i = 0; i < t.word.size(); i++) {
            uppercaseWord += toupper(t.word[i]);
        }

        //Deal with constructs
        if (isKeyWord(uppercaseWord)) {
            cout << "YES" << endl;
        }

        //Deal with normal statements
        else {
            statementQueue = queue<Token>();
            statementQueue.push(t);

            while (!toks.empty() && toks.front().word != ";") {
                t = toks.front();
                toks.pop();
                statementQueue.push(t);
            }

            try {
                op = expTreeBuilder.getExpressionTree(statementQueue);
                this->functions[currentFunction].push_back(op);
            } catch (PostfixError &e) {
                this->errors.push(e);
            }
        }
    }
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Parser::initKeywords() {
    keywords[   "IF"   ] = 1;
    keywords[  "ELSE"  ] = 1;
    keywords[   "DO"   ] = 1;
    keywords[ "WHILE" ]  = 1;
    keywords[  "FOR"  ]  = 1;
    keywords["FOREACH"]  = 1;
    keywords[ "SWITCH" ] = 1;
    keywords[  "CASE"  ] = 1;
    keywords["FUNCTION"] = 1;
    keywords[  "TRY"  ]  = 1;
    keywords[ "CATCH" ]  = 1;
    keywords["FINALLY"]  = 1;
    keywords[ "CLASS" ]  = 1;
}


/****************************************************************************************
 *
 ****************************************************************************************/
short Parser::isKeyWord(string word) {
    return Parser::keywords[word];
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Parser::getTokens(string infix, queue<Token> &result) {
    string s;
    Token t;
    this->infix = infix;
    this->lineNumber = 1;
    this->infixPos = 0;
    while (this->infixPos < this->infix.size()) {
        t = this->getNext();
        if (t.word != "") {
            result.push(t);
        }
    }
}


/****************************************************************************************
 * Creates Token instances for each string, number, word, or operator
 ****************************************************************************************/
Token Parser::getNext() {
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
            while (i < this->infix.size() - 1) {
                if (this->infix[i] == '*' && this->infix[i+1] == '/') {
                    break;
                } else if (this->infix[i] == '\n' || this->infix[i] == '\r') {
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
        if (i == this->infix.size()) {
            throw PostfixError("Unterminated String");
        }
        word += this->infix[i++];
    }


    //Get number
    else if (isnumber(this->infix[i]) //its a number
            || (    //it's a decimal followed by a number
                    this->infix[i] == '.' &&
                    i < this->infix.size() - 1 && //don't walk off the array
                    isnumber(this->infix[i + 1])
                )
            || ( //It's a negative number
                    this->infix[i] == '-'
                    && (
                            (i < this->infix.size()-1 && isnumber(this->infix[i + 1]))
                            ||
                            (i < this->infix.size()-2 && this->infix[i+1] == '.' && isnumber(this->infix[i+2]))
                        )
                )
            )
    {
        type = 'n';
        bool dec = false; //Was a decimal used yet?
        bool first = true; //Is this the first iteration? (Only allow '-' as first char)
        while (isnumber(this->infix[i])
              || (
                      this->infix[i] == '-' &&
                      first
                  )
              || (
                      !dec && this->infix[i] == '.' &&
                      i < this->infix.size() - 1    &&
                      isnumber(this->infix[i + 1])
                  )
              ) {
            if (this->infix[i] == '.') {
                dec = true;
            }
            word += this->infix[i++];
            //If you want to track floats, put IF here and check (dec == true)
            first = false;
        }
    }


    //Get Word
    else if (isalpha(this->infix[i])) {
        type = 'w';
        while ((isalnum(this->infix[i]) || this->infix[i] == '_') &&  i <  this->infix.size())
            word += this->infix[i++];
        if (this->expTreeBuilder.isControlWord(word) && (word == "print" || word == "echo")) {
            type = 'o';
        }
    }


    //Get Operator
    else {
        type = 'o';
        while (i < this->infix.size()) {
            word += this->infix[i++];
            if (this->expTreeBuilder.getOperatorHeirchy(word + this->infix[i]) == 0) {
                break;
            }
        }
        if (word != "" && this->expTreeBuilder.getOperatorHeirchy(word) == 0) {
            throw PostfixError("Unknown operator '"+word+"'");
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
