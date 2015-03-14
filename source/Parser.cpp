#include "Parser.h"
#include <iostream>


using namespace std;


/****************************************************************************************
 *
 ****************************************************************************************/
Parser::Parser() {
    this->expTreeBuilder = ExpressionTreeBuilder();
    this->errors = queue<PostfixError>();
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
    //Nothing to be freed
}


/****************************************************************************************
 *
 * The starting point for the script is the method "main" inside the class "~"
 * The reason the class is called "~" is to avoid having the user declare a class with
 * the same name as the script entry point.
 *
 ****************************************************************************************/
map< string, ClassDefinition* >* Parser::parseTokens(queue<Token> &intoks) {
    this->classes = new map<string, ClassDefinition*>();
    this->currentClass = NULL;
    this->currentMethod = NULL;
    this->isMain = false;

    this->toks = intoks;
    this->statementQueue = queue<Token>();

    Token t;
    Token temp;
    OperationNode* op;
    string lowercaseWord;
    string className;
    this->controlStack = stack<OperationNode*>();
    upcomingElse = false;

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

        //Check for classes
        if (this->currentClass == NULL && lowercaseWord != "main" && lowercaseWord != "class") {
            this->errors.push(PostfixError("Use of global statements is forbidden", t));
            return this->classes;
        } else if (this->currentClass == NULL && lowercaseWord == "main") {
            this->startMain(t);
            continue;
        } else if (this->currentClass == NULL && lowercaseWord == "class") {
            if (this->toks.empty() || this->toks.front().type != 'w') {
                this->errors.push(PostfixError("Expecting class name after 'class' keyword", t));
                continue;
            }
            className = toks.front().word;
            this->toks.pop();
            if (!this->toks.empty() && this->toks.front().word != "{" && this->toks.front().word != "inherits") {
                this->errors.push(PostfixError("Expecting '{' after class name", t));
                continue;
            } else if (!this->toks.empty() && this->toks.front().word == "inherits") {
                if (this->toks.empty() || this->toks.front().type != 'w') {
                    this->errors.push(PostfixError("Expecting class name after 'inherits'", t));
                    continue;
                }
                this->toks.pop();
                this->inheritance[this->toks.front().word] = className;
                this->toks.pop();
                if (!this->toks.empty() && this->toks.front().word != "{") {
                    this->errors.push(PostfixError("Expecting '{' after class name", t));
                    continue;
                }
            }

            this->startClass(className, t);
            this->toks.pop();
        } else if (this->currentMethod == NULL && isKeyWord(t.word) > 1 && isKeyWord(t.word) < 5) {
            this->startProperty(t);
        }

        //Don't allow leading or repeated semicolons
        if (t.word == ";") continue;

        //Deal with constructs
        if (isKeyWord(lowercaseWord)) {
            if (lowercaseWord == "if") {
                //Add condition first
                this->addCondition(true, t);
                //Add if statement to controlStack and add it instructions
                this->addToken(t, true);
            } else if (lowercaseWord == "else") {
                if (!upcomingElse) {
                    this->errors.push(PostfixError("Unexpected 'else' not following 'if'", t));
                }
                //Add else statement to controlStack but not to instructions
                this->addToken(t, false);
            } else if (lowercaseWord == "while") {
                //Save current position
                pos = currentMethod->getInstructionSize() + 1;
                op = this->createJump(pos, true);
                this->controlStack.push(op);
                //Add condition to control stack
                this->addCondition(false, t);
                //Add empty jump
                op = this->createJump(0, false);
                this->controlStack.push(op);
                currentMethod->addInstruction(op);
                //Add while statement to controlStack and add it instructions
                t.word = lowercaseWord;
                this->addToken(t, false);
            } else if (lowercaseWord == "do") {
                if (this->toks.front().word != "{") {
                    this->errors.push(PostfixError("Expecting '{' after 'do'", t));
                }
                //Save current position
                pos = currentMethod->getInstructionSize();
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
            if (this->currentClass == NULL && this->currentMethod == NULL) {
                this->errors.push(PostfixError("Unexpected '}'", t));
            } else if (controlStack.empty() && this->currentMethod != NULL) {
                this->endMethod();
            } else if (controlStack.empty()) {
                this->endClass();
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
                this->addStatement(true, t);
                this->endScope(false);
            } catch (PostfixError &e) {
                this->errors.push(e);
            }
        }
    }

    if (this->currentMethod != NULL) {
        this->errors.push(PostfixError("Expecting '}' to end method", t));
    } else if (this->currentClass != NULL) {
        this->errors.push(PostfixError("Expecting '}' to end class", t));
    } else if (this->classes->find("~") == this->classes->end()) {
        t = Token();
        this->errors.push(PostfixError("Expecting declaration of main", t));
    }

    //Set up inheritance on classes
    map<string, string>::iterator it;
    for (it = inheritance.begin(); it != inheritance.end(); it++) {
        if (this->classes->find(it->second) == this->classes->end()) {
            this->errors.push(PostfixError("Inherited class '"+it->second+"' not found", t));
        }
        (*this->classes)[it->first]->setInheritance((*this->classes)[it->second]);
    }

    return this->classes;
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Parser::startMain(Token &t) {
    string className  = "~";
    string methodName = "main";
    this->isMain = true;

    this->startClass(className, t);
    this->startMethod(methodName, PUBLIC, false, t);

    if (this->toks.front().word != "{") {
        this->errors.push(PostfixError("Expecting '{' after main", t));
    }

    toks.pop();
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Parser::startClass(string &name, Token &t) {
    if (this->classes->find(name) != this->classes->end()) {
        this->errors.push(PostfixError("A class with the name '"+name+"' already exists.", t));
    }
    this->currentClass = new ClassDefinition();
    (*this->classes)[name] = this->currentClass;
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Parser::startMethod(string &name, Visibility visibility, bool isStatic, Token &t) {
    this->currentMethod = new Method(visibility, isStatic);
    this->currentClass->addMethod(name, this->currentMethod, t);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void inline Parser::startProperty(Token &t) {
    Visibility visibility;
    bool isStatic;
    short val = isKeyWord(t.word);

    if (val < 2 || val > 4) {
        cout << "--" << toks.front().word << "--" << endl;
        this->errors.push(PostfixError("Expecting visibility 'public' 'private' or 'protected'", t));
        return;
    }

    switch (val) {
        case 2:
            visibility = PUBLIC;
        break;
        case 3:
            visibility = PRIVATE;
        break;
        default:
            visibility = PROTECTED;
        break;
    }

    if (toks.empty() || isKeyWord(toks.front().word) < 5) {
        this->errors.push(PostfixError("Expecting 'static' or 'dynamic' keyword before property", t));
        return;
    }

    t = toks.front();
    toks.pop();
    isStatic = isKeyWord(t.word) < 6;

    if (toks.empty() || toks.front().type != 'w') {
        this->errors.push(PostfixError("Expecting property name", t));
        return;
    }

    t = toks.front();
    toks.pop();

    if (toks.front().word == ";") {
        Variable v(visibility, isStatic);
        this->currentClass->addProperty(t.word, v, t);
    } else {
        //Add method
        this->startMethod(t.word, visibility, isStatic, t);
        //Get parameters
        this->addMethodParameters(t);
    }
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Parser::endClass() {
    this->currentClass  = NULL;
    this->currentMethod = NULL;
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Parser::endMethod() {
    this->currentMethod = NULL;
    if (this->isMain) {
        this->endClass();
        this->isMain = false;
    }
}


/****************************************************************************************
 *
 ****************************************************************************************/
inline void Parser::addToken(Token &t, bool pushOnFunction) {
    OperationNode* op = new OperationNode();

    t.type = 'o';
    op->operation = t;

    if (pushOnFunction) {
        currentMethod->addInstruction(op);
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

    int parenth = (t.word == "(") - (t.word == ")");

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
        throw PostfixError("Expecting ';' in statement", t);
    } if (!isFor && !this->toks.empty() && this->toks.front().word == ";") {
        this->toks.pop();
    } else if (isFor && t.word != ")") {
        throw PostfixError("Expecting ')' in FOR loop condition", t);
    } else if (isFor) {
        this->toks.pop();
    }
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Parser::addStatement(bool toFunction, Token t) {
    OperationNode* op;

    try {
        op = expTreeBuilder.getExpressionTree(statementQueue);
        if (op == NULL && !toFunction) {
            this->errors.push(PostfixError("Expecting condition", t));
        }
        if (toFunction) {
            currentMethod->addInstruction(op);
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
void Parser::addCondition(bool toFunction, Token t) {
    unsigned int parenths = 1;

    //Are there no more tokens?
    if (toks.empty()) {
        this->errors.push(PostfixError("Unexpected end of file after construct keyword", t));
        return;
    }

    t = toks.front();

    //Is the next token not a parenthesis?
    if (t.word != "(") {
        this->errors.push(PostfixError("Unexpected '" + t.word + "' after construct keyword", t));
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
            this->errors.push(PostfixError("Unexpected ';' in conditional statement", t));
        } else {
            statementQueue.push(t);
        }
    }

    if (parenths > 0) {
        this->errors.push(PostfixError("Unexpected end of file", t));
    }

    this->addStatement(toFunction, t);
}


/****************************************************************************************
 * Add the parameters for a method definition
 * public static foo (xxxxxxxxx) {}
 ****************************************************************************************/
void Parser::addMethodParameters(Token &t) {
    //Remove first parenth
    this->toks.pop();

    //Are there no more tokens?
    if (this->toks.empty()) {
        this->errors.push(PostfixError("Unexpected end of file after method definition", t));
        return;
    }

    bool mustAssign = false;
    short point     = 0;

    //Get the condition
    while (!toks.empty() && toks.front().word != ")") {
        t = toks.front();
        toks.pop();
        point++;
        if ((point == 1 || point == 3) && t.type == 'o') {
            this->errors.push(PostfixError("Expecting value in method definition", t));
        } else if ((point == 2 || point == 4) && (t.word != "=" && t.word != ",")) {
            this->errors.push(PostfixError("Unexpected value in method definition", t));
        } else if (point == 2 && mustAssign && t.word != "=") {
            this->errors.push(PostfixError("Expecting default parameter in method definition", t));
        } else if (point == 1) {
            this->currentMethod->addParameter(t.word);
        } else if (point == 2) {
            if (t.word == "=") {
                mustAssign = true;
            } else {
                point = 0;
            }
        } else if (point == 3) {
            this->currentMethod->addDefault(t);
        } else if (point == 4) {
            point = 0;
        }
    }

    if (this->toks.front().word != ")") {
        this->errors.push(PostfixError("Unexpected end of file", t));
        return;
    }

    this->toks.pop();
    if (this->toks.front().word != "{") {
        this->errors.push(PostfixError("Expecting '{' after method definition", t));
        return;
    }

    this->toks.pop();
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
        sprintf(num, "%lu", currentMethod->getInstructionSize() + upcomingElse);
        controlStack.top()->right->operation.word = num;
        controlStack.pop();
    }

    //If there is an upcoming else, add jump statement
    if (upcomingElse) {
        //Prepare new jump on previous IF statement if branch was taken
        op = createJump(0, false);
        currentMethod->addInstruction(op);
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
    jmp->operation.type = 'o';
    this->controlStack.pop();
    jmp->right = new OperationNode();
    jmp->right->operation = Token();
    jmp->right->operation.type = 'n';
    sprintf(num, "%lu", currentMethod->getInstructionSize());
    jmp->right->operation.word = num;

    //Push on condition
    cond = this->controlStack.top();
    this->controlStack.pop();
    currentMethod->addInstruction(cond);

    //Push on while
    currentMethod->addInstruction(whl);

    //Push on last jump
    jmp = this->controlStack.top();
    jmp->operation.type = 'o';
    this->controlStack.pop();
    currentMethod->addInstruction(jmp);

    //Update while jump
    sprintf(num, "%lu", currentMethod->getInstructionSize());
    whl->operation.word = "if";
    whl->operation.type = 'o';
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
        this->errors.push(PostfixError("Unexpected end before 'while' in 'do while'", t));
        return;
    }

    //Check for following "while"
    t = this->toks.front();
    lowercaseWord = "";
    for (int i = 0; i < t.word.size(); i++) {
        lowercaseWord += tolower(t.word[i]);
    }

    if (lowercaseWord != "while") {
        this->errors.push(PostfixError("Expecting 'while' after 'do'", t));
        return;
    }
    this->toks.pop();

    //Add the condition
    this->addCondition(true, t);

    if (this->toks.front().word != ";") {
        this->errors.push(PostfixError("Expecting ';' after do while loop", t));
    }

    //Add the if
    sprintf(num, "%lu", currentMethod->getInstructionSize() + 2);
    op = new OperationNode();
    op->operation = Token();
    op->operation.word = "if";
    op->operation.type = 'o';
    op->right = new OperationNode();
    op->right->operation = Token();
    op->right->operation.type = 'n';
    op->right->operation.word = num;
    currentMethod->addInstruction(op);

    //Add the jump back
    op = this->controlStack.top();
    op->operation.type = 'o';
    this->controlStack.pop();
    currentMethod->addInstruction(op);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Parser::beginFor(Token &t, string &lowercaseWord) {
    OperationNode* op;
    Token temp;
    int pos;

    if (this->toks.front().word != "(") {
        this->errors.push(PostfixError("Expecting condition in FOR loop", t));
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
            this->addStatement(true, t);
        }
        //Save current position
        pos = currentMethod->getInstructionSize() + 1;
        op = this->createJump(pos, true);
        this->controlStack.push(op);
        //Get condition
        t = this->toks.front();
        this->toks.pop();
        if (t.word != ";") {
            this->getStatement(t, false);
            this->addStatement(false, t);
        } else {
            this->controlStack.push(NULL);
        }
        //Get iteration
        t = this->toks.front();
        this->toks.pop();
        if (t.word != ")") {
            this->getStatement(t, true);
            this->addStatement(false, t);
        } else {
            this->controlStack.push(NULL);
        }
        //Create jump
        op = this->createJump(0, false);
        this->controlStack.push(op);
        currentMethod->addInstruction(op);
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
    jmp->operation.type = 'o';
    this->controlStack.pop();
    bool iterNotNull = this->controlStack.top() != NULL;
    sprintf(num, "%lu", currentMethod->getInstructionSize() + iterNotNull);
    jmp->right = new OperationNode();
    jmp->right->operation = Token();
    jmp->right->operation.type = 'n';
    jmp->right->operation.word = num;

    //Add the iterator statement
    iter = this->controlStack.top();
    this->controlStack.pop();
    if (iter != NULL) {
        currentMethod->addInstruction(iter);
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
    currentMethod->addInstruction(cond);

    //Add the if
    sprintf(num, "%lu", currentMethod->getInstructionSize() + 2);
    fr->operation.word = "if";
    fr->operation.type = 'o';
    fr->right = new OperationNode();
    fr->right->operation = Token();
    fr->right->operation.type = 'n';
    fr->right->operation.word = num;
    currentMethod->addInstruction(fr);

    //Get final jump
    jmp = this->controlStack.top();
    jmp->operation.type = 'o';
    this->controlStack.pop();
    currentMethod->addInstruction(jmp);
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
    //Constructs
    keywords[ "if"      ] = 1;
    keywords[ "else"    ] = 1;
    keywords[ "do"      ] = 1;
    keywords[ "while"   ] = 1;
    keywords[ "for"     ] = 1;

    //Unimplemented keywords
    //keywords[ "foreach" ] = 1;
    //keywords[ "switch"  ] = 1;
    //keywords[ "case"    ] = 1;
    //keywords[ "try"     ] = 1;
    //keywords[ "catch"   ] = 1;
    //keywords[ "finally" ] = 1;

    //Classes
    keywords[ "class"   ] = 1;
    keywords[ "public"    ] = 2;
    keywords[ "private"   ] = 3;
    keywords[ "protected" ] = 4;
    keywords[ "static"    ] = 5;
    keywords[ "dynamic"   ] = 6;
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
