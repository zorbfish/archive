
#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <queue>
#include "process.h"
#include "word.h"

namespace brain {
    class Interpreter;
}

class Semaphore {
public:
    Semaphore();

    void Down( brain::Interpreter& );
    void Up( brain::Interpreter& );

    Semaphore& operator =( const words::Word& );

private:
    words::Word mCount;
    std::deque< brain::Process > mBlocked;
};

#endif