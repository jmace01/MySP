#ifndef SOURCE_VARIABLES_NUMBER_H_
#define SOURCE_VARIABLES_NUMBER_H_

#include "Variable.h"

class Number : public Variable {
    private:
        double value;

    public:
        Number(Visibility visibility, bool isStatic, double value);
        ~Number();

        char getType();
        std::string getTypeString();
        double getNumberValue();
        virtual std::string getStringValue();

        virtual Variable* operator+ (Variable &rhs);
        virtual Variable* operator- (Variable &rhs);
        virtual Variable* operator++ (int);
        virtual Variable* operator-- (int);
        virtual Variable* operator* (Variable &rhs);
        virtual Variable* operator/ (Variable &rhs);
        virtual Variable* operator% (Variable &rhs);
        virtual bool operator== (Variable &rhs);
        virtual bool operator< (Variable &rhs);

        virtual Variable* power(Variable &rhs);
};

#endif
