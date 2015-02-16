#include "OperationNode.h"
#include <iostream>
#include <ctype.h>
#include <queue>
#include <map>
#include "ExpressionTreeBuilder.h"

using namespace std;


/******************************************************************
 *
 ******************************************************************/
ExpressionTreeBuilder::ExpressionTreeBuilder() {
    this->infix              = "";
    this->infixPos           = 0;
    this->lineNumber         = 0;
    this->tempVariableNumber = "";

    this->initializeHierarchy();
}


/******************************************************************
 *
 ******************************************************************/
ExpressionTreeBuilder::~ExpressionTreeBuilder() {
}


/******************************************************************
 *
 ******************************************************************/
OperationNode* ExpressionTreeBuilder::getExpressionTree(string infix, unsigned int lineNumber) throw (PostfixError) {
    this->infix              = infix;
    this->infixPos           = 0;
    this->lineNumber         = lineNumber;
    this->operands           = stack<OperationNode*>();
    this->operators          = stack<Token>();

    bool wasWord = false;
    bool wasFunctionCall = false;
    stack<int> functionParenth = stack<int>();
    OperationNode* result;
    OperationNode* temp;

    queue<Token> toks = this->getTokens();
    if (toks.empty()) return NULL;

    this->validateStatement(toks);

    Token t;
    while (!toks.empty()) {
        t = toks.front();
        toks.pop();

        //Pre Unary operators, such as !, ~ or print
        if (t.type == 'o' && this->isPreUnary(t.word)) {
            operators.push(t);

        //Post Unary Operators, such as ++ or --
        } else if (t.type == 'o' && this->isPostUnary(t.word)) {
            if (operands.size() == 0) {
                cout << "ERROR not enough operands " << t.word << endl;
                continue;
            }

            bool isNotTernary = (t.word != "?");
            operators.push(t);
            this->addOperation(isNotTernary);

        //Normal operators
        } else if (t.type == 'o') {
            while (t.word != "(" && t.word != "["
                    && operators.size() != 0 && operators.top().word != "?"
                    && this->getOperatorHeirchy(operators.top().word) > 0
                    && this->getOperatorHeirchy(t.word) < this->getOperatorHeirchy(operators.top().word)) {
                if (operands.size() < 2) {
                    cout << "ERROR not enough operands " << t.word << endl;
                    break;
                }
                bool isUnary = this->isPreUnary(operators.top().word);
                this->addOperation(isUnary);
            }
            //Function calls
            if (wasWord && t.word == "(") {
                functionParenth.push(1);
            } else if (t.word == "(" && functionParenth.size() > 0) {
                int i = functionParenth.top();
                functionParenth.pop();
                functionParenth.push(++i);
            }
            if ((t.word == ")" || t.word == ",") && operators.size() > 0 && (operators.top().word == "(" || operators.top().word == ",")) {
                //track function parameters
                if (functionParenth.size() > 0) {
                    if (t.word == ")" && functionParenth.top() == 1) {
                        functionParenth.pop();
                        //Add a last parameter?
                        if (!wasFunctionCall) {
                            this->makeParameter();
                            this->operators.pop();
                        }
                        //Chain parameters
                        while(operators.top().word != "(") {
                            if (operators.top().word == ",") {
                                operators.pop();
                                continue;
                            }
                            this->chainParameter();
                        }
                        //Add the function call
                        this->addFunctionCall();
                        operators.pop();
                    } else if (t.word == ")") {
                        int i = functionParenth.top();
                        functionParenth.pop();
                        functionParenth.push(--i);
                    } else if (t.word == ",") {
                        this->makeParameter();
                    }
                } else {
                    operators.pop();
                }
            }
            if (t.word != "(" && operators.size() > 0 && (this->isPreUnary(operators.top().word) || t.word == "]")) {
                bool isNotBracket = (t.word != "]");
                if (operands.size() > 1 || isNotBracket) {
                    this->addOperation(isNotBracket);
                }
            }
            if (t.word != ")" && t.word != "]") {
                operators.push(t);
            }

        //Operands
        } else {
            temp = new OperationNode();
            temp->operation = t;
            operands.push(temp);
        }

        wasFunctionCall = (wasWord && t.word == "(");
        wasWord = (t.type == 'w');
    }

    //Deal with operators still in stack
    while (
            operators.size() > 0 && (
                operands.size() >= 2 || (
                    operands.size() == 1 && (
                        this->isControlWord(operators.top().word) ||
                        this->isPreUnary(operators.top().word)
                    )
                )
            )
          ) {
        if (operators.size() > 0 && (operators.top().word == "(")) {
            operators.pop();
            continue;
        }

        t = operators.top();
        bool preUnary = this->isPreUnary(t.word) || this->isControlWord(t.word);
        this->addOperation(preUnary);
    }

    //The last operand is the root of operation the tree
    result = this->operands.top();
    this->operands.pop();
    return result;
}


/******************************************************************
 *
 ******************************************************************/
void ExpressionTreeBuilder::validateStatement(queue<Token> toks) throw (PostfixError) {
	stack<char> parenths      = stack<char>(); //Parenthesis that are still open
	bool expectingOperator    = false;         //Is it time for an operator?
	bool wasWord              = false;         //Was the last a word? Needed for function calls
	bool wasFunctionOpenning  = false;
	bool wasClosingBacket     = false;
	bool wasClosingParenth    = false;
	stack<int> functParenth   = stack<int>();
	stack<int> ternaryParenth = stack<int>();  //Track ? and :

	Token t;
	while (!toks.empty()) {
		t = toks.front();
		toks.pop();

		//Parenthesis
		if (t.word == "(" || t.word == "[") {
		    if (t.word == "[" && !wasWord && !wasClosingBacket) {
		        throw PostfixError("Illegal use of '[' without an array");
		    }
		    else if (t.word == "(" && wasClosingParenth) {
		        throw PostfixError("Illegal use of '('");
		    }
			parenths.push(t.word[0]);
			expectingOperator = false;
		} else if (t.word == ")" || t.word == "]") {
			if ((t.word == ")" && !wasFunctionOpenning) && (parenths.size() < 1 || !expectingOperator || (t.word == ")" && parenths.top() != '(') || (t.word == "]" && parenths.top() != '['))) {
			    if (t.word == ")")
			        throw PostfixError("Unexpected closing parenthesis");
			    else
			        throw PostfixError("Unexpected closing bracket");
			}
			parenths.pop();
			expectingOperator = true;
		}

		//Unexpected Operators
		else if (expectingOperator && t.type != 'o') {
			throw PostfixError("Unexpected Value " + t.word);
		} else if (!expectingOperator && t.type == 'o' && !this->isPreUnary(t.word) && !this->isControlWord(t.word)) {
			throw PostfixError("Unexpected Operator " + t.word);
		}

		//Other
		else {
		    if (t.word == "?") {
		        ternaryParenth.push(parenths.size());
		    } else if (t.word == ":") {
		        if (ternaryParenth.size() == 0) {
		            throw PostfixError("Unexpected ':' with no preceeding '?'");
		        } else if (ternaryParenth.top() != parenths.size()) {
		            throw PostfixError("Unexpected ':', expecting ')'");
		        }
		        ternaryParenth.pop();
		    } else if (t.word == "&" && (toks.empty() || toks.front().type != 'w')) {
		        throw PostfixError("Illegal use of '&' without variable");
		    }
			expectingOperator = (
				(
					   t.type != 'o'
					|| this->isPostUnary(t.word)
				)
				&& !this->isPreUnary(t.word)
				&& t.word != "?"
				&& t.word != ":"
				&& !isControlWord(t.word)
			);
		}

		wasClosingParenth = (t.word == ")");
		wasClosingBacket = (t.word == "]");
		wasFunctionOpenning = (wasWord && t.word == "(");
		wasWord = (t.type == 'w');
	}

	if (parenths.size() > 0) {
		throw PostfixError("Unclosed parenthesis");
	} else if (ternaryParenth.size() > 0) {
	    throw PostfixError("Unfinished ternary statement requires ':' after '?'");
	} else if (!expectingOperator) {
	    throw PostfixError("Expecting operand to finish statement");
	}
}


/******************************************************************
 *
 ******************************************************************/
queue<Token> ExpressionTreeBuilder::getTokens() {
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
Token ExpressionTreeBuilder::getNext() {
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
            first = false;
        }
    }


    //Get Word
    else if (isalpha(this->infix[i])) {
        type = 'w';
        while ((isalnum(this->infix[i]) || this->infix[i] == '_') &&  i <  this->infix.size())
            word += this->infix[i++];
        if (getOperatorHeirchy(word) == 1) {
            type = 'o';
        }
    }


    //Get Operator
    else {
        type = 'o';
        while (i < this->infix.size()) {
            word += this->infix[i++];
            if (getOperatorHeirchy(word + this->infix[i]) == 0) {
                break;
            }
        }
        if (word != "" && this->getOperatorHeirchy(word) == 0) {
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


/******************************************************************
 *
 ******************************************************************/
void inline ExpressionTreeBuilder::initializeHierarchy() {
    this->opHierarchy = map<string, int>();
    opHierarchy["print"]    = 1;
    opHierarchy["echo"]     = 1;
    opHierarchy["return"]   = 1;
    opHierarchy["break"]    = 1;
    opHierarchy["continue"] = 1;
    opHierarchy["="]  = 2;
    opHierarchy["+="] = 2;
    opHierarchy["-="] = 2;
    opHierarchy["*="] = 2;
    opHierarchy["/="] = 2;
    opHierarchy["%="] = 2;
    opHierarchy["^="] = 2;
    opHierarchy["===="] = 3;
    opHierarchy["==="]  = 3;
    opHierarchy["=="]   = 3;
    opHierarchy["!==="] = 3;
    opHierarchy["!=="]  = 3;
    opHierarchy["!="]   = 3;
    opHierarchy["<"]    = 3;
    opHierarchy["<="]   = 3;
    opHierarchy[">"]    = 3;
    opHierarchy[">="]   = 3;
    opHierarchy["&&"] = 4;
    opHierarchy["||"] = 5;
    opHierarchy["?"] = 6;
    opHierarchy[":"] = 7;
    opHierarchy["."] = 8;
    opHierarchy["-"] = 9;
    opHierarchy["+"] = 9;
    opHierarchy["*"] = 10;
    opHierarchy["/"] = 10;
    opHierarchy["%"] = 10;
    opHierarchy["^"] = 11;
    opHierarchy["++"] = 12;
    opHierarchy["--"] = 12;
    opHierarchy["!"] = 13;
    opHierarchy["~"] = 13;
    opHierarchy["&"] = 13;
    opHierarchy["("] = -1;
    opHierarchy[")"] = -1;
    opHierarchy["["] = -1;
    opHierarchy["]"] = -1;
    opHierarchy[","] = -1;
}

/******************************************************************
 *
 ******************************************************************/
int ExpressionTreeBuilder::getOperatorHeirchy(std::string op) {
    return this->opHierarchy[op];
}


/******************************************************************
 *
 ******************************************************************/
bool ExpressionTreeBuilder::isPostUnary(string op) {
    int h = this->getOperatorHeirchy(op);
    return (h == 12);
}


/******************************************************************
 *
 ******************************************************************/
bool ExpressionTreeBuilder::isPreUnary(string op) {
    int h = this->getOperatorHeirchy(op);
    return (h == 13);
}


/******************************************************************
 *
 ******************************************************************/
bool ExpressionTreeBuilder::isControlWord(string op) {
    int h = this->getOperatorHeirchy(op);
    return (h == 1);
}


/******************************************************************
 * Attaches operands to an operator and puts the result on the
 * operand stack
 *
 * Binary a + b      Unary c++
 *       +               ++
 *     /   \            /
 *   b       a        c
 ******************************************************************/
void ExpressionTreeBuilder::addOperation(bool isUnary) {
    OperationNode* temp = new OperationNode();

    temp->operation = operators.top();
    operators.pop();

    temp->left = operands.top();
    operands.pop();

    if (!isUnary) {
        temp->right = operands.top();
        operands.pop();
    }

    operands.push(temp);
}


/******************************************************************
 * Marks an operand as a parameter
 *
 * foo(bar)
 * P
 *   \
 *     bar
 ******************************************************************/
void inline ExpressionTreeBuilder::makeParameter() {
    //Create new node
    OperationNode* temp = new OperationNode();

    //Create operation token
    temp->operation = Token();
    temp->operation.type = 'o';
    temp->operation.word = "P";

    //Add parameter value to right
    //Right side is important for execution order!
    temp->right = this->operands.top();
    this->operands.pop();

    //Push on operand stack
    this->operands.push(temp);

    //Put marker on operator stack
    Token t = Token();
    t.type = 'P';
    this->operators.push(t);
}


/******************************************************************
 * Chains multiple function parameters together
 * The first parameter in the list will be the lowest on the tree
 *
 * Example: foo(a, b, c)
 *         P
 *       /   \
 *     P      c
 *   /   \
 * P       b
 *   \
 *     a
 ******************************************************************/
void inline ExpressionTreeBuilder::chainParameter() {
    //Get the first operand
    OperationNode* temp = this->operands.top();
    this->operands.pop();

    //Get leftmost node to add parameter to
    OperationNode* t = temp;
    while (t->left != NULL) {
        t = t->left;
    }

    //Add the next operand
    t->left = this->operands.top();
    this->operands.pop();

    //Push the operand back on the stack
    this->operands.push(temp);

    //Remove marker from operator stack
    this->operators.pop();
}


/******************************************************************
 *
 * foo(bar)
 *
 *     C
 *   /   \
 * P       foo
 *   \
 *    bar
 ******************************************************************/
void inline ExpressionTreeBuilder::addFunctionCall() {
    //Create the call node
    OperationNode* temp = new OperationNode();

    //Add the operation to the call node
    temp->operation = Token();
    temp->operation.type = 'o';
    temp->operation.word = "C";

    //If there are parameters, add them to the left
    //It is important they be on the left!
    if (this->operands.top()->operation.word == "P") {
        temp->left = this->operands.top();
        this->operands.pop();
    }

    //Add the function name to the right
    temp->right = this->operands.top();
    this->operands.pop();

    //Add the call node
    this->operands.push(temp);
}
