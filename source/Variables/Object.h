#ifndef SOURCE_VARIABLES_OBJECT_H_
#define SOURCE_VARIABLES_OBJECT_H_

#include <map>
#include <string>
#include "../ClassDefinition.h"
#include "../Method.h"
#include "Variable.h"

class Object: public Variable {
    private:
        ClassDefinition* classDef;
        std::map<std::string, Variable**> properties;

    public:
        Object(Visibility visibility, ClassDefinition* cd);
        Object(Visibility visibility, Object* obj);
        ~Object();
        char getType();
        std::string getTypeString();
        double getNumberValue();
        virtual std::string getStringValue();
        virtual bool getBooleanValue();
        virtual Variable* getProperty(std::string index);
        virtual Method* getMethod(std::string index);
        ClassDefinition* getClass();
};

#endif
