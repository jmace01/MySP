#ifndef SOURCE_VARIABLES_ARRAY_H_
#define SOURCE_VARIABLES_ARRAY_H_

#include <map>
#include "Variable.h"

class Array: public Variable {
    private:
        std::map<std::string, Variable**> values;

    public:
        Array(Visibility visibility);
        ~Array();
        char getType();
        std::string getTypeString();
        double getNumberValue();
        virtual std::string getStringValue();
        virtual bool getBooleanValue();
        virtual Variable* getArrayValue(std::string index);
};

#endif
