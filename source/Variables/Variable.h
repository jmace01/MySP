#ifndef SOURCE_VARIABLES_VARIABLE_H_
#define SOURCE_VARIABLES_VARIABLE_H_

#include <string>

class Variable {
    protected:
        bool isPrivate;

    public:
        Variable();
        virtual ~Variable();
        bool getIsPrivate();

        virtual char getType() = 0;
        virtual float getNumberValue() = 0;
        virtual std::string getStringValue() = 0;
        virtual Variable* getArrayValue() = 0;
        virtual void getObjectValue() = 0;
};

#endif
