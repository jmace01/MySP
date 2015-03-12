#ifndef SOURCE_VARIABLES_VARIABLE_H_
#define SOURCE_VARIABLES_VARIABLE_H_

#include <string>



//Visibility types
enum Visibility {PRIVATE, PUBLIC, PROTECTED, TEMP};


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
        Visibility visibility;
        bool isStatic;
        Variable** pointer;

    public:
        Variable(Visibility visibility, bool isStatic);
        virtual ~Variable();
        Visibility getVisibility();
        void setPointer(Variable** pointer);
        Variable** getPointer();

        virtual char getType();
        virtual std::string getTypeString();
        virtual double getNumberValue();
        virtual std::string getStringValue();
        virtual Variable* getArrayValue();

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
        virtual Method* getMethod(std::string, bool isStatic);
        virtual Variable* getPropery(std::string, bool isStatic);
};

#endif
