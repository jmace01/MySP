#ifndef SOURCE_OPTIONS_H_
#define SOURCE_OPTIONS_H_

#include <string>

class Options {

    private:
        bool timeExecution;
        bool runUnitTests;
        std::string filename;

    public:
        Options();
        ~Options();
        bool parseOptions(int argc, char** argv);
        bool getTimeExecution();
        bool getRunUnitTests();
        std::string getFilename();
};

#endif
