#ifndef SOURCE_STRUCTUREPARSER_H_
#define SOURCE_STRUCTUREPARSER_H_

#include <map>
#include <string>
#include "ExpressionTreeBuilder.h"


/****************************************************************************************
 *
 ****************************************************************************************/
class StructureParser {
    private:
        ExpressionTreeBuilder expTreeBuilder;
        static std::map<std::string, short> keywords;

    public:
        StructureParser();
        ~StructureParser();
        void initKeywords();
        short static isKeyWord(std::string word);
};

#endif
