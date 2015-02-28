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
    Token temp;
    OperationNode* op;
    string lowercaseWord;
    this->controlStack = stack<OperationNode*>();
    upcomingElse = false;

    this->inFunction = false;

    int pos = 0;

    while (!toks.empty()) {
        //Get token off queue
        t = toks.front();
        toks.pop();

        //Convert to lower-case
        lowercaseWord = "";
        for (int i = 0; i < t.word.size(); i++) {
            lowercaseWord += tolower(t.word[i]);
        }

        //Check for functions
        if (!inFunction && lowercaseWord != "function") {
            this->errors.push(PostfixError("Use of global statements is forbidden"));
        } else if (inFunction && lowercaseWord == "function") {
            this->errors.push(PostfixError("Cannot declare function inside of a function"));
        } else if (!inFunction && lowercaseWord == "function") {
            this->startFunction();
            continue;
        }

        //Don't allow leading or repeated semicolons
        if (t.word == ";") continue;

        //Deal with constructs
        if (isKeyWord(lowercaseWord)) {
            if (lowercaseWord == "if") {
                //Add condition first
                this->addCondition(true);
                //Add if statement to controlStack and add it instructions
                this->addToken(t, true);
            } else if (lowercaseWord == "else") {
                if (!upcomingElse) {
                    this->errors.push(PostfixError("Unexpected 'else' not following 'if'"));
                }
                //Add else statement to controlStack but not to instructions
                this->addToken(t, false);
            } else if (lowercaseWord == "while") {
                //Save current position
                pos = (*this->functions)[currentFunction].size() + 1;
                op = this->createJump(pos, true);
                this->controlStack.push(op);
                //Add condition to control stack
                this->addCondition(false);
                //Add empty jump
                op = this->createJump(0, false);
                this->controlStack.push(op);
                (*this->functions)[currentFunction].push_back(op);
                //Add while statement to controlStack and add it instructions
                t.word = lowercaseWord;
                this->addToken(t, false);
            } else if (lowercaseWord == "do") {
                if (this->toks.front().word != "{") {
                    this->errors.push(PostfixError("Expecting '{' after 'do'"));
                }
                //Save current position
                pos = (*this->functions)[currentFunction].size();
                op = this->createJump(pos, true);
                this->controlStack.push(op);
                //Save the do
                t.word = lowercaseWord;
                this->addToken(t, false);
            } else if (lowercaseWord == "for") {
                this->beginFor(t, lowercaseWord);
            }
        }

        //Deal with end of scopes
        else if (t.word == "}") {
            if (inFunction && controlStack.empty()) {
                this->inFunction = false;
                continue;
            } else if (!inFunction || controlStack.empty() || controlStack.top()->operation.type != 'o') {
                this->errors.push(PostfixError("Unexpected '}'"));
            } else if (!controlStack.empty() && controlStack.top()->operation.word == "if") {
                //Set up jump on previous IF statement if branch was not taken
                this->endScope(true);
            } else if (!controlStack.empty() && controlStack.top()->operation.word == "else") {
                //Set up jump on previous IF statement if branch was not taken
                controlStack.pop();
                this->endScope(true);
            } else if (!controlStack.empty() && controlStack.top()->operation.word == "while") {
                this->endScope(true);
            } else if (!controlStack.empty() && controlStack.top()->operation.word == "do") {
                this->endScope(true);
            } else if (!controlStack.empty() && controlStack.top()->operation.word == "for") {
                this->endScope(true);
            }
        }

        //Deal with normal statements
        else {
            try {
                this->getStatement(t, false);
                this->addStatement(true);
                this->endScope(false);
            } catch (PostfixError &e) {
                this->errors.push(e);
            }
        }
    }

    return this->functions;
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Parser::startFunction() {
    if (this->toks.empty() || this->toks.front().type != 'w') {
        this->errors.push(PostfixError("Expecting function name after function keyword"));
    }

    this->currentFunction = this->toks.front().word;

    toks.pop();
    toks.pop();
    toks.pop();
    toks.pop();

    this->inFunction = true;
}


/****************************************************************************************
 *
 ****************************************************************************************/
inline void Parser::addToken(Token &t, bool pushOnFunction) {
    OperationNode* op = new OperationNode();

    t.type = 'o';
    op->operation = t;

    if (pushOnFunction) {
        (*this->functions)[currentFunction].push_back(op);
    }

    //Save the location to modify the jump later
    controlStack.push(op);
    this->markScoped(op);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Parser::getStatement(Token &t, bool isFor) {
    statementQueue = queue<Token>();
    statementQueue.push(t);

    int parenth = 0;

    while (!toks.empty() && toks.front().word != ";") {
        t = toks.front();
        if (t.word == "(") {
            parenth++;
        } else if (t.word == ")") {
            parenth--;
        }
        if (parenth < 0) {
            break;
        }
        toks.pop();
        statementQueue.push(t);
    }

    if (!isFor && parenth < 0) {
        throw PostfixError("Expecting ';' in statement");
    } if (!isFor && !this->toks.empty() && this->toks.front().word == ";") {
        this->toks.pop();
    } else if (isFor && t.word != ")") {
        throw PostfixError("Expecting ')' in FOR loop condition");
    } else if (isFor) {
        this->toks.pop();
    }
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Parser::addStatement(bool toFunction) {
    OperationNode* op;

    try {
        op = expTreeBuilder.getExpressionTree(statementQueue);
        if (op == NULL && !toFunction) {
            this->errors.push(PostfixError("Expecting condition"));
        }
        if (toFunction) {
            (*this->functions)[currentFunction].push_back(op);
        } else {
            this->controlStack.push(op);
        }
    } catch (PostfixError &e) {
        this->errors.push(e);
    }

    statementQueue = queue<Token>();
}


/****************************************************************************************
 * Add the conditional part of a construct.
 * while (xxxx), if (xxxx)
 ****************************************************************************************/
void Parser::addCondition(bool toFunction) {
    unsigned int parenths = 1;

    //Are there no more tokens?
    if (toks.empty()) {
        this->errors.push(PostfixError("Unexpected end of file after construct keyword"));
        return;
    }

    Token t = toks.front();

    //Is the next token not a parenthesis?
    if (t.word != "(") {
        this->errors.push(PostfixError("Unexpected '" + t.word + "' after construct keyword"));
        return;
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
            this->errors.push(PostfixError("Unexpected ';' in conditional statement"));
        } else {
            statementQueue.push(t);
        }
    }

    if (parenths > 0) {
        this->errors.push(PostfixError("Unexpected end of file"));
    }

    this->addStatement(toFunction);
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
        if (this->controlStack.top()->operation.word == "while") {
            this->endWhile();
            continue;
        } else if (this->controlStack.top()->operation.word == "do") {
            this->endDoWhile();
            continue;
        } else if (this->controlStack.top()->operation.word == "for") {
            this->endFor();
            continue;
        }
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
        op = createJump(0, false);
        (*this->functions)[currentFunction].push_back(op);
        controlStack.push(op);
    }
}


/****************************************************************************************
 *
 *
 * For the while loop:
 * WHILE (A) {
 *     B;
 * }
 *
 * Currently on the controlStack would be the values
 * jmp-1
 * A
 * jmp-
 * while-
 *
 * Becomes:
 *  ______________________________
 * |   0   | 1 | 2 |   3  |  4    |
 * |=======|===|===|======|=======|
 * | jmp   |   |   | if   | jmp   |
 * |    \  | B | A |   \  |    \  |
 * |     2 |   |   |    5 |     1 |
 *  ------------------------------
 ****************************************************************************************/
inline void Parser::endWhile() {
    OperationNode* whl;
    OperationNode* jmp;
    OperationNode* cond;

    char num[30];

    //Modify while token
    whl = this->controlStack.top();
    this->controlStack.pop();

    //Modify jump
    jmp = this->controlStack.top();
    this->controlStack.pop();
    jmp->right = new OperationNode();
    jmp->right->operation = Token();
    jmp->right->operation.type = 'n';
    sprintf(num, "%lu", (*this->functions)[currentFunction].size());
    jmp->right->operation.word = num;

    //Push on condition
    cond = this->controlStack.top();
    this->controlStack.pop();
    (*this->functions)[currentFunction].push_back(cond);

    //Push on while
    (*this->functions)[currentFunction].push_back(whl);

    //Push on last jump
    jmp = this->controlStack.top();
    this->controlStack.pop();
    (*this->functions)[currentFunction].push_back(jmp);

    //Update while jump
    sprintf(num, "%lu", (*this->functions)[currentFunction].size());
    whl->operation.word = "if";
    whl->right = new OperationNode();
    whl->right->operation = Token();
    whl->right->operation.type = 'n';
    whl->right->operation.word = num;
}


/****************************************************************************************
 *
 *
 * For the do-while loop:
 * DO {
 *     B;
 * } WHILE (A);
 *
 * Currently on the controlStack would be the value
 * jump-0
 * do
 *
 * Becomes:
 *  ______________________
 * | 0 | 1 |   2  |  3    |
 * |===|===|======|=======|
 * |   |   | if   | jmp   |
 * | B | A |   \  |    \  |
 * |   |   |    4 |     0 |
 *  ----------------------
 ****************************************************************************************/
void Parser::endDoWhile() {
    Token t;
    string lowercaseWord;
    OperationNode* op;
    char num[30];

    this->controlStack.pop();

    //Make sure there are more tokens
    if (this->toks.empty()) {
        this->errors.push(PostfixError("Unexpected end before 'while' in 'do while'"));
        return;
    }

    //Check for following "while"
    t = this->toks.front();
    lowercaseWord = "";
    for (int i = 0; i < t.word.size(); i++) {
        lowercaseWord += tolower(t.word[i]);
    }

    if (lowercaseWord != "while") {
        this->errors.push(PostfixError("Expecting 'while' after 'do'"));
        return;
    }
    this->toks.pop();

    //Add the condition
    this->addCondition(true);

    if (this->toks.front().word != ";") {
        this->errors.push(PostfixError("Expecting ';' after do while loop"));
    }

    //Add the if
    sprintf(num, "%lu", (*this->functions)[currentFunction].size() + 2);
    op = new OperationNode();
    op->operation = Token();
    op->operation.word = "if";
    op->operation.type = 'o';
    op->right = new OperationNode();
    op->right->operation = Token();
    op->right->operation.type = 'n';
    op->right->operation.word = num;
    (*this->functions)[currentFunction].push_back(op);

    //Add the jump back
    op = this->controlStack.top();
    this->controlStack.pop();
    (*this->functions)[currentFunction].push_back(op);
}


void Parser::beginFor(Token &t, string &lowercaseWord) {
    OperationNode* op;
    Token temp;
    int pos;

    if (this->toks.front().word != "(") {
        this->errors.push(PostfixError("Expecting condition in FOR loop"));
        return;
    }
    try {
        temp = Token();
        temp.word = "";
        this->addToken(temp, false);
        temp = t;
        this->toks.pop();
        //Get initial
        t = this->toks.front();
        this->toks.pop();
        if (t.word != ";") {
            this->getStatement(t, false);
            this->addStatement(true);
        }
        //Save current position
        pos = (*this->functions)[currentFunction].size() + 1;
        op = this->createJump(pos, true);
        this->controlStack.push(op);
        //Get condition
        t = this->toks.front();
        this->toks.pop();
        if (t.word != ";") {
            this->getStatement(t, false);
            this->addStatement(false);
        } else {
            this->controlStack.push(NULL);
        }
        //Get iteration
        t = this->toks.front();
        this->toks.pop();
        if (t.word != ")") {
            this->getStatement(t, true);
            this->addStatement(false);
        } else {
            this->controlStack.push(NULL);
        }
        //Create jump
        op = this->createJump(0, false);
        this->controlStack.push(op);
        (*this->functions)[currentFunction].push_back(op);
        //Save the for
        temp.word = lowercaseWord;
        this->addToken(temp, false);
    } catch (PostfixError &e) {
        if (e.msg == "Expecting ';' in statement") {
            e.msg = "Expecting ';' in FOR loop condition";
        }
        this->errors.push(e);
        while (this->controlStack.top()->operation.word != "") {
            this->controlStack.pop();
        }
        this->controlStack.pop();
    }
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Parser::endFor() {
    OperationNode* fr;
    OperationNode* jmp;
    OperationNode* iter;
    OperationNode* cond;
    char num[30];

    //Get the for token
    fr = this->controlStack.top();
    this->controlStack.pop();

    //Get the initial jmp
    jmp = this->controlStack.top();
    this->controlStack.pop();
    bool iterNotNull = this->controlStack.top() != NULL;
    sprintf(num, "%lu", (*this->functions)[currentFunction].size() + iterNotNull);
    jmp->right = new OperationNode();
    jmp->right->operation = Token();
    jmp->right->operation.type = 'n';
    jmp->right->operation.word = num;

    //Add the iterator statement
    iter = this->controlStack.top();
    this->controlStack.pop();
    if (iter != NULL) {
        (*this->functions)[currentFunction].push_back(iter);
    }

    //Add the condition statement
    cond = this->controlStack.top();
    this->controlStack.pop();
    if (cond == NULL) {
        cond = new OperationNode();
        cond->operation = Token();
        cond->operation.type = 'n';
        cond->operation.word = "1";
    }
    (*this->functions)[currentFunction].push_back(cond);

    //Add the if
    sprintf(num, "%lu", (*this->functions)[currentFunction].size() + 2);
    fr->operation.word = "if";
    fr->operation.type = 'o';
    fr->right = new OperationNode();
    fr->right->operation = Token();
    fr->right->operation.type = 'n';
    fr->right->operation.word = num;
    (*this->functions)[currentFunction].push_back(fr);

    //Get final jump
    jmp = this->controlStack.top();
    this->controlStack.pop();
    (*this->functions)[currentFunction].push_back(jmp);
}


/****************************************************************************************
 *
 ****************************************************************************************/
OperationNode* Parser::createJump(unsigned long pos, bool includePos) {
    char num[30];
    OperationNode* op = new OperationNode();
    op->operation = Token();
    op->operation.type = 's';
    op->operation.word = "jmp";

    //Is there a specific value specified?
    if (includePos) {
        sprintf(num, "%lu", pos);
        op->right = new OperationNode();
        op->right->operation.type = 'n';
        op->right->operation.word = num;
    }

    return op;
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
queue<PostfixError> Parser::getErrors() {
    return this->errors;
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Parser::clearErrors() {
    while(!this->errors.empty()) {
        this->errors.pop();
    }
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
            this->errors.push(PostfixError("Unterminated String"));
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
            this->errors.push(PostfixError("Unknown operator '"+word+"'"));
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
