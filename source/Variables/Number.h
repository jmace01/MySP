#ifndef SOURCE_VARIABLES_NUMBER_H_
#define SOURCE_VARIABLES_NUMBER_H_

#include "Variable.h"

class Number : public Variable {
    private:
        float value;

    public:
        Number(Visibility visibility, bool isStatic, int value);
        ~Number();

        char getType();
        float getNumberValue();
};

#endif
