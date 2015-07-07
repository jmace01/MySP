#include "Options.h"
#include <iostream>

using namespace std;


Options::Options() {
    this->timeExecution = false;
    this->runUnitTests  = false;
}

Options::~Options() {
    //
}

bool Options::parseOptions(int argc, char** argv) {
    string flag;

    for (int i = 1; i < argc; i++) {
        //Is this a flag?
        if (argv[i][0] == '-') {
            flag = "";
            char* c = argv[i];
            for (c++; *c != '\0'; c++) {
                flag += *c;
            }
            if (flag == "test") {
                this->runUnitTests = true;
            } else if (flag == "time") {
                this->timeExecution = true;
            }
        }
        //Is this the filename?
        else if (this->filename == "") {
            this->filename = argv[i];
        }
        //Skip it
        else {
            continue;
        }
    }

    return true;
}

bool Options::getTimeExecution() {
    return this->timeExecution;
}

bool Options::getRunUnitTests() {
    return this->runUnitTests;
}

string Options::getFilename() {
    return this->filename;
}
