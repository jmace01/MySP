#ifndef SOURCE_VARIABLES_STRING_H_
#define SOURCE_VARIABLES_STRING_H_


#include <string.h>
#include "Variable.h"


class String : public Variable {
    private:
        std::string value;

    public:
        String(Visibility visibility, bool isStatic, std::string &value);
        ~String();
        std::string getStringValue();
};

#endif
