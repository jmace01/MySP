#ifndef SOURCE_VARIABLES_VARIABLE_H_
#define SOURCE_VARIABLES_VARIABLE_H_

#include <string>



//Visibility types
enum Visibility {PRIVATE, PUBLIC, PROTECTED, INHERIT_PRIVATE, TEMP, CONST, PARAMETER};


//This is an unfortunate circular reference
class Method;


enum ERROR_LEVEL {WARNING, ERROR, FATAL};


//RuntimeError struct for throwing
struct RuntimeError {
        RuntimeError(std::string msg, ERROR_LEVEL level) : msg(msg), level(level), line(0) {}
        std::string msg;
        ERROR_LEVEL level;
        int line;
};



//Variable class definition
class Variable {
    protected:
        Visibility preConstVisibility;
        Visibility visibility;
        Variable** pointer;

    public:
        Variable(Visibility visibility);
        virtual ~Variable();
        Visibility getVisibility();
        void setPointer(Variable** pointer);
        Variable** getPointer();
        void makeConstant();
        void makeNonConstant();

        virtual char getType();
        virtual std::string getTypeString();
        virtual double getNumberValue();
        virtual std::string getStringValue();
        virtual bool getBooleanValue();
        virtual Variable* getArrayValue(std::string index);

        Variable* operator= (Variable &rhs);
        virtual Variable* operator+ (Variable &rhs);
        virtual Variable* operator- (Variable &rhs);
        virtual Variable* operator++ (int);
        virtual Variable* operator-- (int);
        virtual Variable* operator* (Variable &rhs);
        virtual Variable* operator/ (Variable &rhs);
        virtual Variable* operator% (Variable &rhs);
        virtual Variable* operator[] (int i);
        virtual bool operator== (Variable &rhs);
        virtual bool operator< (Variable &rhs);
        virtual bool operator> (Variable &rhs);

        virtual Variable* power(Variable &rhs);
        virtual Variable* concat(Variable &rhs);
        virtual Method* getMethod(std::string index);
        virtual Variable* getProperty(std::string index);
};

#endif
