#ifndef SOURCE_TOKEN_H_
#define SOURCE_TOKEN_H_

#include <string>

struct Token{
    Token() : type(0), line(0), isUnary(false) {}
    std::string word;
    char type;
    int line;
    bool isUnary;
};


#endif /* SOURCE_TOKEN_H_ */
