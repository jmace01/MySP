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
map< string, vector<OperationNode*> >* Parser::parseText(string infix) {
    this->functions = new map< string, vector<OperationNode*> >();
    this->toks = queue<Token>();
    this->statementQueue = queue<Token>();
    this->getTokens(infix, toks);

    Token t;
    string lowercaseWord;
    OperationNode* op; //Generic holder for statements
    this->controlStack = stack<OperationNode*>();
    upcomingElse = false;

    while (!toks.empty()) {
        //Get token off queue
        t = toks.front();
        toks.pop();

        //Don't allow leading or repeated semicolons
        if (t.word == ";") continue;

        //Convert to lower-case
        lowercaseWord = "";
        for (int i = 0; i < t.word.size(); i++) {
            lowercaseWord += tolower(t.word[i]);
        }

        //Deal with constructs
        if (isKeyWord(lowercaseWord)) {
            if (lowercaseWord == "if") {
                //Add condition first
                this->addCondition();
                //Add if statement
                t.type = 'o';
                op = new OperationNode();
                op->operation = t;
                (*this->functions)[currentFunction].push_back(op);
                //Save the location to modify the jump later
                controlStack.push(op);
                this->markScoped(op);
            } else if (lowercaseWord == "else") {
                if (!upcomingElse) {
                    throw PostfixError("Unexpected 'else' not following 'if'");
                }
                //Add else statement
                t.type = 'o';
                op = new OperationNode();
                op->operation = t;
                controlStack.push(op);
                this->markScoped(op);
            }
        }

        //Deal with end of scopes
        else if (t.word == "}") {
            if (controlStack.empty() || controlStack.top()->operation.type != 'o') {
                throw PostfixError("Unexpected '}'");
            } else if (!controlStack.empty() && controlStack.top()->operation.word == "if") {
                //Set up jump on previous IF statement if branch was not taken
                this->endScope(true);
            } else if (!controlStack.empty() && controlStack.top()->operation.word == "else") {
                //Set up jump on previous IF statement if branch was not taken
                controlStack.pop();
                this->endScope(true);
            }
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

            this->addStatement();
            this->endScope(false);
        }
    }

    return this->functions;
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Parser::addStatement() {
    OperationNode* op;

    try {
        op = expTreeBuilder.getExpressionTree(statementQueue);
        (*this->functions)[currentFunction].push_back(op);
    } catch (PostfixError &e) {
        //cout << e.msg << endl;
        this->errors.push(e);
    }

    statementQueue = queue<Token>();
}


/****************************************************************************************
 * Add the conditional part of a construct.
 * while (xxxx), if (xxxx)
 ****************************************************************************************/
void Parser::addCondition() {
    unsigned int parenths = 1;

    //Are there no more tokens?
    if (toks.empty()) {
        throw PostfixError("Unexpected end of file after construct keyword");
    }

    Token t = toks.front();

    //Is the next token not a parenthesis?
    if (t.word != "(") {
        throw PostfixError("Unexpected '" + t.word + "' after construct keyword");
    }

    toks.pop();

    //Get the condition
    while (parenths > 0 && !toks.empty()) {
        t = toks.front();
        toks.pop();
        if (t.word == "(") {
            parenths++;
        } else if (t.word == ")") {
            parenths--;
        } else if (t.word == ";") {
            throw PostfixError("Unexpected ';' in conditional statement");
        } else {
            statementQueue.push(t);
        }
    }

    if (parenths > 0) {
        throw PostfixError("Unexpected end of file");
    }

    this->addStatement();
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Parser::markScoped(OperationNode* op) {
    //Check if there is a curly brace next
    if (toks.front().word == "{") { //Scoped
        toks.pop();
    } else { //Shorthand
        op->operation.type = 's'; //'s' to mark as shorthand
    }
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Parser::endScope(bool setFirst) {
    char num[30];
    OperationNode* op;

    //If ";" is at the front, pop it to get next token
    if (toks.front().word == ";")
        toks.pop();

    //Is there an else statement coming?
    upcomingElse = !controlStack.empty() && controlStack.top()->operation.word == "if"
                    && !toks.empty() && toks.front().word == "else";

    //Update and pop off completed control statements
    while (!this->controlStack.empty() &&
                (
                    setFirst ||
                    this->controlStack.top()->operation.type == 's'
                ) && !(upcomingElse && this->controlStack.top()->operation.word == "jmp")
            )
    {
        setFirst = false;
        controlStack.top()->operation.type = 'o';
        controlStack.top()->right = new OperationNode();
        controlStack.top()->right->operation = Token();
        controlStack.top()->right->operation.type = 'n';
        sprintf(num, "%lu", (*this->functions)[currentFunction].size() + upcomingElse);
        controlStack.top()->right->operation.word = num;
        controlStack.pop();
    }

    //If there is an upcoming else, add jump statement
    if (upcomingElse) {
        //Prepare new jump on previous IF statement if branch was taken
        op = new OperationNode();
        op->operation = Token();
        op->operation.type = 's';
        op->operation.word = "jmp";
        (*this->functions)[currentFunction].push_back(op);
        controlStack.push(op);
    }
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Parser::initKeywords() {
    keywords[   "if"   ] = 1;
    keywords[  "else"  ] = 1;
    keywords[   "do"   ] = 1;
    keywords[ "while" ]  = 1;
    keywords[  "for"  ]  = 1;
    keywords["foreach"]  = 1;
    keywords[ "switch" ] = 1;
    keywords[  "case"  ] = 1;
    keywords["function"] = 1;
    keywords[  "try"  ]  = 1;
    keywords[ "catch" ]  = 1;
    keywords["finally"]  = 1;
    keywords[ "class" ]  = 1;
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
        if (this->expTreeBuilder.isControlWord(word) && (word == "print" || word == "echo" || word == "return")) {
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
