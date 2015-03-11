#include "String.h"


using namespace std;


/****************************************************************************************
 *
 ****************************************************************************************/
String::String(Visibility visibility, bool isStatic, string &value)
    : Variable(visibility, isStatic)
{
    if (value[0] == '"' || value[0] == '\'') {
        this->value = value.substr(1, value.length() - 2);
    } else {
        this->value = value;
    }
}


/****************************************************************************************
 *
 ****************************************************************************************/
String::~String() {
    //
}


/****************************************************************************************
 *
 ****************************************************************************************/
char String::getType() {
    return 's';
}


/****************************************************************************************
 *
 ****************************************************************************************/
string String::getTypeString() {
    return "string";
}


/****************************************************************************************
 *
 ****************************************************************************************/
string String::getStringValue() {
    return this->value;
}


/****************************************************************************************
 *
 ****************************************************************************************/
Variable* String::concat(Variable &rhs) {
    string v = this->value + rhs.getStringValue();
    Variable* result = new String(TEMP, false, v);
    return result;
}
