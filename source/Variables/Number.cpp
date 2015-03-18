#include "Number.h"
#include <math.h>


using namespace std;


/****************************************************************************************
 *
 ****************************************************************************************/
Number::Number(Visibility visibility, double value)
    : Variable(visibility)
{
    this->value = value;
}


/****************************************************************************************
 *
 ****************************************************************************************/
Number::~Number() {
    //Nothing to free
}


/****************************************************************************************
 *
 ****************************************************************************************/
char Number::getType() {
    return 'n';
}


/****************************************************************************************
 *
 ****************************************************************************************/
string Number::getTypeString() {
    return "number";
}


/****************************************************************************************
 *
 ****************************************************************************************/
double Number::getNumberValue() {
    return this->value;
}


/****************************************************************************************
 *
 ****************************************************************************************/
string Number::getStringValue() {
    char val[30];

    //Can this value not be an int?
    if (this->value != (int) this->value) {
        sprintf(val, "%g", this->value);
    }
    //Else print as an int
    else {
        sprintf(val, "%i", (int) this->value);
    }

    return string(val);
}


/****************************************************************************************
 *
 ****************************************************************************************/
bool Number::getBooleanValue() {
    return (this->value != 0);
}


/****************************************************************************************
 *
 ****************************************************************************************/
Variable* Number::operator+ (Variable &rhs) {
    return new Number(TEMP, this->value + rhs.getNumberValue());
}


/****************************************************************************************
 *
 ****************************************************************************************/
Variable* Number::operator- (Variable &rhs) {
    return new Number(TEMP, this->value - rhs.getNumberValue());
}


/****************************************************************************************
 *
 ****************************************************************************************/
Variable* Number::operator++ (int) {
    this->value += 1;
    return new Number(TEMP, this->value - 1);
}


/****************************************************************************************
 *
 ****************************************************************************************/
Variable* Number::operator-- (int) {
    this->value -= 1;
    return new Number(TEMP, this->value + 1);
}


/****************************************************************************************
 *
 ****************************************************************************************/
Variable* Number::operator* (Variable &rhs) {
    return new Number(TEMP, this->value * rhs.getNumberValue());
}


/****************************************************************************************
 *
 ****************************************************************************************/
Variable* Number::operator/ (Variable &rhs) {
    if (rhs.getNumberValue() == 0) {
        throw RuntimeError("Division by 0", WARNING);
    }
    return new Number(TEMP, this->value / rhs.getNumberValue());
}


/****************************************************************************************
 *
 ****************************************************************************************/
Variable* Number::operator% (Variable &rhs) {
    return new Number(TEMP, (int) this->value % (int) rhs.getNumberValue());
}


/****************************************************************************************
 *
 ****************************************************************************************/
Variable* Number::power(Variable &rhs) {
    return new Number(TEMP, pow(this->value, rhs.getNumberValue()));
}
