#include "String.h"

using namespace std;

String::String(Visibility visibility, bool isStatic, string &value)
    : Variable(visibility, isStatic)
{
    this->value = value;
}

String::~String() {
    //
}

