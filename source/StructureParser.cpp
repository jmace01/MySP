#include "StructureParser.h"

using namespace std;

// #yolo -- turn off all error messages and warnings
// #selfie -- turn off wornings
// #whereisthelove -- set precision

/****************************************************************************************
 *
 ****************************************************************************************/
StructureParser::StructureParser() {
    this->expTreeBuilder = ExpressionTreeBuilder();
    if (StructureParser::keywords.empty()) {
        initKeywords();
    }
}

//Initialize keywords map
map<string, short> StructureParser::keywords = map<string, short>();


/****************************************************************************************
 *
 ****************************************************************************************/
StructureParser::~StructureParser() {
    //
}


/****************************************************************************************
 *
 ****************************************************************************************/
void StructureParser::initKeywords() {
    keywords[   "IF"   ] = 1;
    keywords[  "ELSE"  ] = 1;
    keywords[   "DO"   ] = 1;
    keywords[ "WHILE" ]  = 1;
    keywords[  "FOR"  ]  = 1;
    keywords["FOREACH"]  = 1;
    keywords[ "SWITCH" ] = 1;
    keywords[  "CASE"  ] = 1;
    keywords["FUNCTION"] = 1;
    keywords[  "TRY"  ]  = 1;
    keywords[ "CATCH" ] = 1;
    keywords["FINALLY"] = 1;
    keywords[ "CLASS" ] = 1;
}


/****************************************************************************************
 *
 ****************************************************************************************/
short StructureParser::isKeyWord(string word) {
    return StructureParser::keywords[word];
}
