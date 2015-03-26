#include <queue>
#include "ExpressionTreeBuilder.h"
#include "Tokenizer.h"


using namespace std;


/****************************************************************************************
 *
 ****************************************************************************************/
Tokenizer::Tokenizer() {
    this->lineNumber = 0;
    this->infixPos   = 0;
    this->input = NULL;
}


/****************************************************************************************
 *
 ****************************************************************************************/
Tokenizer::~Tokenizer() {
    //
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Tokenizer::getTokens(string infix, queue<Token> &result) {
    string s;
    Token t;
    this->infix = infix;
    this->lineNumber = 1;
    this->infixPos = 0;
    bool wasOp = true;
    while (this->infixPos < this->infix.size()) {
        t = this->getNext(wasOp);
        if (t.word != "") {
            result.push(t);
        }
    }
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Tokenizer::eatWhitespace(int &i) {
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
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Tokenizer::eatComments(int &i) {
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
}


/****************************************************************************************
 * Creates Token instances for each string, number, word, or operator
 ****************************************************************************************/
Token Tokenizer::getNext(bool &wasOp) {
    //Initialize variable
    string word = "";
    char type;

    //Use "i" instead of "this->infixPos"
    int i = this->infixPos;

    Token t;


    //While there are comments or whitespace, eat them
    while (
            this->infix[i] == ' '  ||
            this->infix[i] == '\r' ||
            this->infix[i] == '\n' ||
            this->infix[i] == '\t' ||
            (
                this->infix[i] == '/' &&
                (
                    this->infix[i + 1] == '/' ||
                    this->infix[i+1] == '*'
                )
            )
          )
    {
        this->eatWhitespace(i);
        this->eatComments(i);
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
            t.line = line;
            throw PostfixError("Unterminated String", t);
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
                    this->infix[i] == '-' && wasOp
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
                      first &&
                      wasOp
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
    else if (isalpha(this->infix[i]) || this->infix[i] == '_') {
        type = 'w';
        while ((isalnum(this->infix[i]) || this->infix[i] == '_') &&  i <  this->infix.size())
            word += this->infix[i++];
        if (ExpressionTreeBuilder::isControlWord(word) && (word == "print" || word == "echo" || word == "return")) {
            type = 'o';
        }
    }


    //Get Operator
    else {
        type = 'o';
        while (i < this->infix.size()) {
            word += this->infix[i++];
            if (ExpressionTreeBuilder::getOperatorHeirchy(word + this->infix[i]) == 0) {
                break;
            }
        }
        if (word != "" && ExpressionTreeBuilder::getOperatorHeirchy(word) == 0) {
            t.line = line;
            throw PostfixError("Unknown operator '"+word+"'", t);
        }
    }


    //Save position
    this->infixPos = i;

    //Create the return token
    t = Token();
    t.word = word;
    t.line = line;
    t.type = type;

    wasOp = (t.type == 'o');

    //Return the token
    return t;
}




















/****************************************************************************************
 *
 ****************************************************************************************/
void Tokenizer::getTokens(istream* stream, queue<Token> &result) {
    this->input = stream;
    Token t;
    this->lineNumber = 1;
    bool wasOp = true;
    char currentChar;

    input->get(currentChar);
    while (this->input->good()) {
        t = this->getNextToken(wasOp, currentChar);
        if (t.word != "") {
            result.push(t);
        }
    }
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Tokenizer::eatWhitespace(char &currentChar) {
    while ( input->good() && (
            currentChar == ' '  ||
            currentChar == '\r' ||
            currentChar == '\n' ||
            currentChar == '\t'
          )) {
        if (currentChar == '\n') {
            this->lineNumber++;
        }
        input->get(currentChar);
    }
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Tokenizer::eatComments(char &currentChar) {
    if (currentChar == '/' && (input->peek() == '/' || input->peek() == '*')) {
        input->get(currentChar);
        if (currentChar == '/') { // for "//" comments
            input->get(currentChar);
            while (currentChar != '\n' && input->good()) {
                input->get(currentChar);
            }
            //this->lineNumber++;
        } else { //for "/**/" comments
            input->get(currentChar);
            while (input->good()) {
                if (currentChar == '*' && input->peek() == '/') {
                    break;
                } else if (currentChar == '\n') {
                    this->lineNumber++;
                }
                input->get(currentChar);
            }
            input->get(currentChar);
            input->get(currentChar);
        }
    }
}


/****************************************************************************************
 *
 ****************************************************************************************/
char Tokenizer::getSlashedChar(char c) {
    char r;
    switch (c) {
        case 'r':
            r = '\r';
        break;
        case 'n':
            r = '\n';
        break;
        case 't':
            r = '\t';
        break;
        case 'b':
            r = '\b';
        break;
        default:
            r = c;
    }
    return r;
}


/****************************************************************************************
 * Creates Token instances for each string, number, word, or operator
 ****************************************************************************************/
Token Tokenizer::getNextToken(bool &wasOp, char &currentChar) {

    //Initialize variable
    string word = "";
    char type;
    Token t;

    //While there are comments or whitespace, eat them
    while (
            input->good() && (
            currentChar == ' '  ||
            currentChar == '\r' ||
            currentChar == '\n' ||
            currentChar == '\t' ||
            (
                currentChar == '/' &&
                (
                    input->peek() == '/' ||
                    input->peek() == '*'
                )
            )
          ))
    {
        this->eatWhitespace(currentChar);
        this->eatComments(currentChar);
    }

    //Save the line number for the token
    int line = this->lineNumber;


    //Get String
    if (currentChar == '"' || currentChar == '\'')
    {
        type = 's';
        char delimiter = currentChar;
        bool slash = false;
        char slashedChar;
        do
        {
            if (delimiter == '"' && slash) {
                slashedChar = getSlashedChar(currentChar);
                word[word.length() - 1] = slashedChar;
                slash = false;
            } else {
                slash = (!slash && currentChar == '\\');
                word += currentChar;
            }
            input->get(currentChar);
        }
        while ((currentChar != delimiter || slash) && input->good());
        if (!input->good()) {
            t.line = line;
            throw PostfixError("Unterminated String", t);
        }
        word += currentChar;
        input->get(currentChar);
    }


    //Get number
    else if (isnumber(currentChar) //its a number
            || (    //it's a decimal followed by a number
                    currentChar == '.' &&
                    input->good() && //don't walk off the array
                    isnumber(input->peek())
                )
            || ( //It's a negative number
                    currentChar == '-' && wasOp
                    && (
                            (input->good() && isnumber(input->peek()))
                            ||
                            (input->good() && input->peek() == '.')
                        )
                )
            )
    {
        type = 'n';
        bool dec = false; //Was a decimal used yet?
        bool first = true; //Is this the first iteration? (Only allow '-' as first char)
        while (isnumber(currentChar)
              || (
                      currentChar == '-' &&
                      first &&
                      wasOp
                  )
              || (
                      !dec && currentChar == '.' &&
                      input->good()    &&
                      isnumber(input->peek())
                  )
              ) {
            if (currentChar == '.') {
                dec = true;
            }
            word += currentChar;
            input->get(currentChar);
            //If you want to track floats, put IF here and check (dec == true)
            first = false;
        }
    }


    //Get Word
    else if (isalpha(currentChar) || currentChar == '_') {
        type = 'w';
        while ((isalnum(currentChar) || currentChar == '_') &&  input->good()) {
            word += currentChar;
            input->get(currentChar);
        }
        if (ExpressionTreeBuilder::isControlWord(word) && (word == "print" || word == "echo" || word == "return")) {
            type = 'o';
        }
    }


    //Get Operator
    else {
        type = 'o';
        while (input->good()) {
            word += currentChar;
            input->get(currentChar);
            if (ExpressionTreeBuilder::getOperatorHeirchy(word + currentChar) == 0) {
                break;
            }
        }
        if (word != "" && ExpressionTreeBuilder::getOperatorHeirchy(word) == 0) {
            t.line = line;
            throw PostfixError("Unknown operator '"+word+"'", t);
        }
    }

    //Create the return token
    t = Token();
    t.word = word;
    t.line = line;
    t.type = type;

    wasOp = (t.type == 'o');

    //Return the token
    return t;
}
