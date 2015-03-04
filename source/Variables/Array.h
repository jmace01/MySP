#ifndef SOURCE_VARIABLES_ARRAY_H_
#define SOURCE_VARIABLES_ARRAY_H_

#include "Variable.h"

class Array: public Variable {
    public:
        Array(Visibility visibility, bool isStatic);
        virtual ~Array();
};

#endif
