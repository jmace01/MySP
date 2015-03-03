#ifndef SOURCE_VARIABLES_VARIABLE_H_
#define SOURCE_VARIABLES_VARIABLE_H_

#include <string>
#include "../Method.h"

class Variable {
    protected:
        Visibility visibility;
        bool isStatic;

    public:
        Variable(Visibility visibility, bool isStatic);
        virtual ~Variable();
        Visibility getVisibility();

        virtual char getType();
        virtual float getNumberValue();
        virtual std::string getStringValue();
        virtual Variable* getArrayValue();
        virtual void getObjectValue();
};

#endif
