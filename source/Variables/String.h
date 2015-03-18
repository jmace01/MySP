#ifndef SOURCE_VARIABLES_STRING_H_
#define SOURCE_VARIABLES_STRING_H_


#include <string.h>
#include "Variable.h"


class String : public Variable {
    private:
        std::string value;

    public:
        String(Visibility visibility, std::string &value);
        ~String();
        char getType();
        std::string getStringValue();
        bool getBooleanValue();
        Variable* concat(Variable &rhs);
        std::string getTypeString();
};

#endif
