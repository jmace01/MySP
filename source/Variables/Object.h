#ifndef SOURCE_VARIABLES_OBJECT_H_
#define SOURCE_VARIABLES_OBJECT_H_

#include "Variable.h"

class Object: public Variable {
    public:
        Object(Visibility visibility, bool isStatic);
        ~Object();
};

#endif
