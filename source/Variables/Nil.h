#ifndef SOURCE_VARIABLES_NIL_H_
#define SOURCE_VARIABLES_NIL_H_

#include <string>
#include "Variable.h"

class Nil : public Variable {
    public:
        Nil(Visibility visibility);
        virtual ~Nil();
        char getType();
        std::string getStringValue();
        bool getBooleanValue();
        std::string getTypeString();
};

#endif
