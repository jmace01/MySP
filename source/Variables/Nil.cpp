#include "Nil.h"


using namespace std;


/****************************************************************************************
 *
 ****************************************************************************************/
Nil::Nil(Visibility visibility)
    : Variable(visibility)
{
}


/****************************************************************************************
 *
 ****************************************************************************************/
Nil::~Nil() {
}


/****************************************************************************************
 *
 ****************************************************************************************/
char Nil::getType() {
    return '0';
}


/****************************************************************************************
 *
 ****************************************************************************************/
string Nil::getTypeString() {
    return "null";
}


/****************************************************************************************
 *
 ****************************************************************************************/
string Nil::getStringValue() {
    return "NULL";
}


/****************************************************************************************
 *
 ****************************************************************************************/
bool Nil::getBooleanValue() {
    return false;
}