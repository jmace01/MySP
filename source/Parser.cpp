/*/////////////////////////////////////// !! ////////////////////////////////////////////
 *
 * FILE:
 *     Parser.cpp
 *
 * DESCRIPTION:
 *     Parses tokens from the Tokenizer class and creates ClassDefinitions that can be
 *     executed by the Executor class.
 *
 * AUTHOR:
 *     Jason Mace
 *
 *
 * Copyright 2015 by Jason Mace
 *
 */////////////////////////////////////// !! ////////////////////////////////////////////


#include "Parser.h"
#include <iostream>


using namespace std;


/****************************************************************************************
 * Parser::Parser
 *
 * Description:
 *     Constructor. Initialize member variables.
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
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
 * Parser::~Parser
 *
 * Description:
 *     Destructor. Nothing needs to be freed.
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
 ****************************************************************************************/
Parser::~Parser() {
    //Nothing to be freed
}


/****************************************************************************************
 * Parser::parseTokens
 *
 * Description:
 *     This method takes the input tokens and returns the binary expression tree for the
 *     statement. After calling this method, the Parser::getErrors method should be
 *     called to check if any error were found during the parsing of the input tokens.
 *
 * Note:
 *     The starting point for the script execution is the method "main" inside the class
 *     "~" The reason the class is called "~" is to avoid having the user declare a class
 *     with the same name as the script entry point.
 *
 * Inputs:
 *     queue<Token> &intoks : The tokens to parse. This should come from the Tokenizer
 *         class.
 *
 * Outputs:
 *     map< string, ClassDefinition* >* : The class definitions generated from the input
 *         tokens.
 ****************************************************************************************/
map< string, ClassDefinition* >* Parser::parseTokens(queue<Token> &intoks) {
    //This is the return value, which is the class definition created from the tokens.
    this->classes = new map<string, ClassDefinition*>();

    this->currentClass = NULL; //The class currently being defined with tokens
    this->currentMethod = NULL; //The method currently being defined with tokens
    this->isMain = false; //Is the current method main?
    this->inheritance = map<string, string>(); //The inheritance for classes

    this->toks = intoks; //The input tokens to use
    this->statementQueue = queue<Token>(); //Holds tokens to pass to ExpressionTreeBuilder

    Token t; //Current token from toks
    Token temp; //Temporary token holder
    OperationNode* op;
    string lowercaseWord; //For case insensitive keywords
    string className;
    this->controlStack = stack<OperationNode*>(); //Control word stack
    this->exceptions = stack<string>();
    upcomingElse = false; //Check if an ELSE is follow an IF
    bool expectingCatch = false;
    bool expectingFinally = false;

    //Only show one invalid property message
    bool invalidProperty = false;

    int pos = 0;

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
        } else if (this->currentMethod == NULL && t.word != "}") {
            if (!invalidProperty) {
                invalidProperty = true;
                this->errors.push(PostfixError("Expecting method or property definition", t));
            }
            continue;
        }

        //Only one invalid property message
        invalidProperty = false;

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
                pos = currentMethod->getInstructionTreeSize() + 1;
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
                pos = currentMethod->getInstructionTreeSize();
                op = this->createJump(pos, true);
                this->controlStack.push(op);
                //Save the do
                t.word = lowercaseWord;
                this->addToken(t, false);
            } else if (lowercaseWord == "for") {
                this->beginFor(t, lowercaseWord);
            } else if (lowercaseWord == "try") {
                //Make sure try is scoped
                if (this->toks.front().word != "{") {
                    this->errors.push(PostfixError("Expecting '{' after 'try'", t));
                }
                //Add try to stack
                t.word = lowercaseWord;
                this->addToken(t, true);
            } else if (lowercaseWord == "catch") {
                //Make sure a catch is expected
                if (!expectingCatch) {
                    this->errors.push(PostfixError("Unexpected 'catch'", t));
                }
                //Make sure catch has arguments
                this->catchParams();
                //Make sure catch is scoped
                if (this->toks.front().word != "{") {
                    this->errors.push(PostfixError("Expecting '{' after 'catch'", t));
                }
                //Add catch to stack
                t.word = lowercaseWord;
                this->addToken(t, true);
                if (!this->toks.empty() && this->toks.front().word == "{") {
                    this->errors.push(PostfixError("Unexpected '{'", t));
                }
                this->addToken(t, true);
                this->controlStack.top()->operation.type = 'w';
                expectingCatch = false;
            } else if (lowercaseWord == "finally") {
                //Make sure a catch is expected
                if (!expectingFinally) {
                    this->errors.push(PostfixError("Unexpected 'finally'", t));
                }
                //Make sure catch is scoped
                if (this->toks.front().word != "{") {
                    this->errors.push(PostfixError("Expecting '{' after 'finally'", t));
                }
                //Add catch to stack
                t.word = lowercaseWord;
                this->addToken(t, true);
                expectingFinally = false;
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
            } else if (!controlStack.empty() && controlStack.top()->operation.word == "try") {
                this->endScope(true);
                if (toks.front().word != "catch") {
                    this->errors.push(PostfixError("Expected 'catch' after 'try'", toks.front()));
                } else {
                    expectingCatch = true;
                }
            } else if (!controlStack.empty() && controlStack.top()->operation.word == "catch") {
                this->endScope(true);
                expectingFinally = (toks.front().word == "finally");
                expectingCatch = (toks.front().word == "catch");
            } else if (!controlStack.empty() && controlStack.top()->operation.word == "finally") {
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
        if (this->classes->find(it->first) == this->classes->end()) {
            this->errors.push(PostfixError("Inherited class '"+it->first+"' not found", t));
            continue;
        }
        (*this->classes)[it->second]->setInheritance((*this->classes)[it->first]);
    }

    return this->classes;
}


/****************************************************************************************
 * Parser::startMain
 *
 * Description:
 *     Start the main method.
 *     This is a special case because main is not defined like the other classes.
 *
 * Inputs:
 *     Token &t : The token that starts main. This is needed for error messages in case
 *         of syntax errors.
 *
 * Outputs:
 *     None
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
 * Parser::startClass
 *
 * Description:
 *     The method created a new class in the classDefinition map and points currentClass
 *     to it so the class can be defined and returned when parsing finished.
 *
 * Inputs:
 *     string &name : The name of the new class to start
 *     Token &t : The token that starts the class. This is needed for error messages in
 *         case of syntax errors.
 *
 * Outputs:
 *     None
 ****************************************************************************************/
void Parser::startClass(string &name, Token &t) {
    if (this->classes->find(name) != this->classes->end()) {
        this->errors.push(PostfixError("A class with the name '"+name+"' already exists.", t));
    }
    this->currentClass = new ClassDefinition();
    (*this->classes)[name] = this->currentClass;
}


/****************************************************************************************
 * Parser::startMethod
 *
 * Description:
 *     Creates a new Method instance in the current class definition and sets
 *     currentMethod to point to it.
 *
 * Inputs:
 *     string &name : The name of the method to begin.
 *     Visibility visibility : The visibility of the new method
 *         (public, private, protected)
 *     bool isStatic : If true, the method will be marked as static
 *     Token &t : The token that starts the method. This is needed for error messages in
 *         case of syntax errors.
 *
 * Outputs:
 *     None
 ****************************************************************************************/
void Parser::startMethod(string &name, Visibility visibility, bool isStatic, Token &t) {
    this->currentMethod = new Method(visibility, isStatic);
    this->currentClass->addMethod(name, this->currentMethod, t);
}


/****************************************************************************************
 * Parser::startProperty
 *
 * Description:
 *     Gets a property or a method of a class. The property should be in the form:
 *     [public|private|protected] [static|dynamic] [{name}];     //Member variable
 *     [public|private|protected] [static|dynamic] [{name}]() {} //Method
 *     Example:
 *         private dynamic foo;
 *         public static bar() { / *. . .* / }
 *
 * Inputs:
 *     Token &t : The first keyword of a property, which should be 'public', 'private',
 *         or protected.
 *
 * Outputs:
 *     None
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
        Variable v(visibility);
        if (isStatic) {
            this->currentClass->addStaticProperty(t.word, v, t);
        } else {
            this->currentClass->addProperty(t.word, v, t);
        }
    } else {
        //Add method
        this->startMethod(t.word, visibility, isStatic, t);
        //Get parameters
        this->addMethodParameters(t);
    }
}


/****************************************************************************************
 * Parser::endClass
 *
 * Description:
 *     Ends the definition of a class so that another can be started.
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
 ****************************************************************************************/
void Parser::endClass() {
    this->currentClass  = NULL;
    this->currentMethod = NULL;
}


/****************************************************************************************
 * endMethod
 *
 * Description:
 *     Ends a method definition so that another class attribute can be started.
 *     If the current method is main, it ends the class as well.
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
 ****************************************************************************************/
void Parser::endMethod() {
    this->currentMethod = NULL;
    if (this->isMain) {
        this->endClass();
        this->isMain = false;
    }
}


/****************************************************************************************
 * Parser::addToken
 *
 * Description:
 *     Creates a new operation node from a token and adds it to the controlStack. If
 *     pushOnFunction is true, the operation is added to the method definition.
 *
 *     This is used for control structure keywords such as IF, ELSE, FOR, WHILE, etc.
 *
 * Inputs:
 *     Token &t : The token to create an operation node from.
 *     bool pushOnFunction : If true, the operation will be added to currentMethod.
 *
 * Outputs:
 *     None
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
 * Parser::getStatement
 *
 * Description:
 *     Gets an infix statement of tokens to pass to the ExpressionTreeBuilder to generate
 *     a binary expression tree. FOR loops are a special case as the last statement ends
 *     with a ")" instead of a ";".
 *
 * Inputs:
 *     Token &t : The first token of the statement
 *     bool isFor : If true, a ")" will be expected to terminate the statement.
 *         Otherwise, a ";" will be expected.
 *
 * Outputs:
 *     None
 ****************************************************************************************/
void Parser::getStatement(Token &t, bool isFor) {
    statementQueue = queue<Token>();
    statementQueue.push(t);

    int parenth = (t.word == "(") - (t.word == ")");

    while (!toks.empty() && toks.front().word != ";") {
        t = toks.front();
        if (t.word == "{" || t.word == "}") {
            throw PostfixError("Unexpected '" + t.word + "' in statement", t);
        } else if (t.word == "(") {
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
 * Parser::addStatement
 *
 * Description:
 *     Passes the statementQueue to the ExpressionTreeBuilder to get the binary
 *     expression tree.
 *
 * Inputs:
 *     bool toFunction : If true, the resulting tree will be put on the currentMethod.
 *         Otherwise, the resulting tree will be put on the controlStack.
 *     Token t : The current token, used for error messages.
 *
 * Outputs:
 *     None
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
 * Parser::addCondition
 *
 * Description:
 *     Adds the conditional part of a construct.
 *     while (xxxx), if (xxxx)
 *
 * Inputs:
 *     bool toFunction : If true, the resulting tree will be put on the currentMethod.
 *         Otherwise, the resulting tree will be put on the controlStack.
 *     Token t : The current token, used for error messages.
 *
 * Outputs:
 *     None
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
 * Parser::addMethodParameters
 *
 * Description:
 *     Add the parameters for a method definition
 *     public static foo (xxxxxxxxx) {}
 *
 * Inputs:
 *     Token &t : The current token, used for error messages.
 *
 * Outputs:
 *     None
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
 * Parser::markScoped
 *
 * Description:
 *     Marks whether a construct is scoped or not based on if the next token is a "{".
 *     while(xxxx) ; is not scoped.
 *     while(xxxx) { ... } is scoped
 *
 * Inputs:
 *     OperationNode* op : The operation to mark
 *
 * Outputs:
 *     None
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
 * Parser::endScope
 *
 * Description:
 *     Ends appropriate constructs when needed.
 *     For example, WHILE (xxxx) { ... } on the last "}" the construct should be finished
 *     and jumps and conditions dealt with and put on the currentMethod.
 *
 * Inputs:
 *     bool setFirst : If the operation is scoped (has "{" and "}") this should be true.
 *         This allows the first scoped construct to be closed. Otherwise only non-
 *         scoped constructs will be closed.
 *
 * Outputs:
 *     None
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
                ) && !(upcomingElse && this->controlStack.top()->operation.word == ":jmp")
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
        } else if (this->controlStack.top()->operation.word == "try") {
            this->endTry();
            continue;
        }  else if (this->controlStack.top()->operation.word == "catch") {
            this->endCatch();
            continue;
        } else if (this->controlStack.top()->operation.word == "finally") {
            this->endFinally();
            continue;
        }
        controlStack.top()->operation.type = 'o';
        controlStack.top()->right = new OperationNode();
        controlStack.top()->right->operation = Token();
        controlStack.top()->right->operation.type = 'n';
        sprintf(num, "%lu", currentMethod->getInstructionTreeSize() + upcomingElse);
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
 * Parser::endWhile
 *
 * Description:
 *     For the while loop:
 *     WHILE (A) {
 *         B;
 *     }
 *
 *     Currently on the controlStack would be the values
 *     jmp-1
 *     A
 *     jmp-
 *     while-
 *
 *     Becomes:
 *      ______________________________
 *     |   0   | 1 | 2 |   3  |  4    |
 *     |=======|===|===|======|=======|
 *     | jmp   |   |   | if   | jmp   |
 *     |    \  | B | A |   \  |    \  |
 *     |     2 |   |   |    5 |     1 |
 *      ------------------------------
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
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
    sprintf(num, "%lu", currentMethod->getInstructionTreeSize());
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
    sprintf(num, "%lu", currentMethod->getInstructionTreeSize());
    whl->operation.word = "if";
    whl->operation.type = 'o';
    whl->right = new OperationNode();
    whl->right->operation = Token();
    whl->right->operation.type = 'n';
    whl->right->operation.word = num;
}


/****************************************************************************************
 * Parser::endDoWhile
 *
 * Description:
 *     For the do-while loop:
 *     DO {
 *         B;
 *     } WHILE (A);
 *
 *     Currently on the controlStack would be the values
 *     jump-0
 *     do
 *
 *     Becomes:
 *      ______________________
 *     | 0 | 1 |   2  |  3    |
 *     |===|===|======|=======|
 *     |   |   | if   | jmp   |
 *     | B | A |   \  |    \  |
 *     |   |   |    4 |     0 |
 *      ----------------------
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
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
    sprintf(num, "%lu", currentMethod->getInstructionTreeSize() + 2);
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
 * Parser::beginFor
 *
 * Description:
 *     Begins the FOR statement by preparing the initial part (A), the condition (B) and
 *     the iteration (C) as well as the initial jump.
 *     FOR (A; B; C) { ... }
 *
 * Inputs:
 *     Token &t : The current token, used for errors.
 *     string &lowercaseWord : The lower case "for".
 *
 * Outputs:
 *     None
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
        pos = currentMethod->getInstructionTreeSize() + 1;
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
 * Parser::endFor
 *
 * Description:
 *     Ends the FOR loop.
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
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
    sprintf(num, "%lu", currentMethod->getInstructionTreeSize() + iterNotNull);
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
    sprintf(num, "%lu", currentMethod->getInstructionTreeSize() + 2);
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
 * Parser::endTry
 *
 * Description:
 *     Ends a Try statement.
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
 ****************************************************************************************/
void Parser::endTry() {
    char num[30];

    OperationNode* op = this->controlStack.top();

    sprintf(num, "%lu", currentMethod->getInstructionTreeSize());
    op->right = new OperationNode();
    op->right->operation.type = 'n';
    op->right->operation.word = num;

    if (this->toks.front().word != "catch") {
        this->controlStack.pop();
    }
}


/****************************************************************************************
 * Parser::catchParams
 *
 * Description:
 *     Gets the arguments for a catch statement
 *     catch (Exception e)
 *           ^^^^^^^^^^^^^
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
 ****************************************************************************************/
void Parser::catchParams() {
    Token t = this->toks.front();
    this->toks.pop();
    if (t.word != "(") {
        errors.push(PostfixError("Expected '(' after catch",t));
        return;
    }

    Token type = this->toks.front();
    this->toks.pop();
    if (type.type != 'w') {
        errors.push(PostfixError("Expected exception class type", t));
        return;
    }
    this->exceptions.push(type.word);

    Token var = this->toks.front();
    this->toks.pop();
    if (var.type != 'w') {
        errors.push(PostfixError("Expected variable after exception type",t));
        return;
    }
    this->exceptions.push(var.word);

    t = this->toks.front();
    this->toks.pop();
    if (t.word != ")") {
        errors.push(PostfixError("Expected ')' after catch variable ",t));
        return;
    }
}


/****************************************************************************************
 * Parser::endCatch
 *
 * Description:
 *     Ends a Catch statement.
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
 ****************************************************************************************/
void Parser::endCatch() {
    char num[30];

    if (this->exceptions.size() < 2 || this->controlStack.size() < 2) {
        while(!this->controlStack.empty() &&
                (
                    this->controlStack.top()->operation.word == "catch" ||
                    this->controlStack.top()->operation.word == "try"
                )
        ) {
            this->controlStack.pop();
        }
        return;
    }

    OperationNode* op2 = this->controlStack.top();
    this->controlStack.pop();
    OperationNode* op1 = this->controlStack.top();
    this->controlStack.pop();
    string var = this->exceptions.top();
    this->exceptions.pop();
    string type = this->exceptions.top();
    this->exceptions.pop();

    sprintf(num, "%lu", currentMethod->getInstructionTreeSize() + 1);
    op1->right = new OperationNode();
    op1->right->operation.type = 'n';
    op1->right->operation.word = num;
    op1->left = new OperationNode();
    op1->left->operation.type = 'w';
    op1->left->operation.word = type;

    sprintf(num, "%lu", currentMethod->getInstructionTreeSize() + 1);
    op2->operation.word = "exception";
    op2->right = new OperationNode();
    op2->right->operation.type = 'n';
    op2->right->operation.word = num;
    op2->left = new OperationNode();
    op2->left->operation.type = 'w';
    op2->left->operation.word = var;

    if (this->toks.front().word != "catch") {
        while (!this->controlStack.empty() && this->controlStack.top()->operation.word == "exception") {
            this->controlStack.top()->right->operation.word = num;
            this->controlStack.pop();
        }
        if (!this->controlStack.empty() && this->controlStack.top()->operation.word == "try") {
            //Add finally location to try
            OperationNode* op = new OperationNode();
            op->operation.type = 'n';
            op->operation.word = num;
            this->controlStack.top()->left = op;
            this->controlStack.pop();
        }
    } else {
        this->controlStack.push(op2);
    }
}


/****************************************************************************************
 * Parser::endFinally
 *
 * Description:
 *     Ends a Finally statement.
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
 ****************************************************************************************/
void Parser::endFinally() {
    char num[30];

    OperationNode* op = this->controlStack.top();

    sprintf(num, "%lu", currentMethod->getInstructionTreeSize() + 1);
    op->right = new OperationNode();
    op->right->operation.type = 'n';
    op->right->operation.word = num;

    this->controlStack.pop();
}


/****************************************************************************************
 * Parser::createJump
 *
 * Description:
 *     //
 *
 * Inputs:
 *     unsigned long pos :
 *     bool includePos :
 *
 * Outputs:
 *     OperationNode* :
 ****************************************************************************************/
OperationNode* Parser::createJump(unsigned long pos, bool includePos) {
    char num[30];
    OperationNode* op = new OperationNode();
    op->operation = Token();
    op->operation.type = 's';
    op->operation.word = ":jmp";

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
 * Parser::initKeywords
 *
 * Description:
 *     Initializes the keyword map.
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
 ****************************************************************************************/
void Parser::initKeywords() {
    //Constructs
    keywords[ "if"      ] = 1;
    keywords[ "else"    ] = 1;
    keywords[ "do"      ] = 1;
    keywords[ "while"   ] = 1;
    keywords[ "for"     ] = 1;
    keywords[ "try"     ] = 1;
    keywords[ "catch"   ] = 1;
    keywords[ "finally" ] = 1;

    //Unimplemented keywords
    //keywords[ "foreach" ] = 1;
    //keywords[ "switch"  ] = 1;
    //keywords[ "case"    ] = 1;

    //Classes
    keywords[ "class"   ] = 1;
    keywords[ "public"    ] = 2;
    keywords[ "private"   ] = 3;
    keywords[ "protected" ] = 4;
    keywords[ "static"    ] = 5;
    keywords[ "dynamic"   ] = 6;
}


/****************************************************************************************
 * Parser::isKeyWord
 *
 * Description:
 *     Returns the value of the input word in the map. If the value is not 0, it is a
 *     keyword.
 *
 * Inputs:
 *     string word : The word to check.
 *
 * Outputs:
 *     None
 ****************************************************************************************/
short Parser::isKeyWord(string word) {
    return Parser::keywords[word];
}


/****************************************************************************************
 * Parser::getErrors
 *
 * Description:
 *     Returns the error message generated from parsing tokens.
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     queue<PostfixError> : The error messages from the Parser::parseTokens call.
 ****************************************************************************************/
queue<PostfixError> Parser::getErrors() {
    return this->errors;
}


/****************************************************************************************
 * Parser::clearErrors
 *
 * Description:
 *     Removed the error messages so another batch of tokens can be parsed.
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
 ****************************************************************************************/
void Parser::clearErrors() {
    while(!this->errors.empty()) {
        this->errors.pop();
    }
}
