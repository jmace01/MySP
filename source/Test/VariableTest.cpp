#include "Test.h"
#include "../Variables/Array.h"
#include "../Variables/Number.h"
#include "../Variables/Object.h"
#include "../Variables/String.h"
#include "../Variables/Variable.h"
#include <iostream>


using namespace std;


void Test::testVariables() {
    cout << "Starting Variable Unit Test" << endl;
    clock_t timer = clock();

    Variable* a = new Variable(PUBLIC, false);
    Variable* b = new Number(PUBLIC, false, 0);

    int i = 0;
    int passed = 0;

    b = (*b + *b);
    cout << b->getTypeString() << endl;
    b = (*b = *a);
    cout << b->getTypeString() << endl;

    delete a;
    delete b;

    float targetSpeed = 0.0001 * i;
    float time = (float)(clock() - timer) / CLOCKS_PER_SEC;
    cout << "    Passed " << passed << "/" << i << " tests in " << time << " seconds" << endl;
    if (time > targetSpeed) cout << "    -- Processed too slow [should be " << targetSpeed << "]" << endl;
}
