
#include "semaphore.h"

#include "interpreter.h"
using brain::Interpreter;

#include "process.h"
using brain::Process;

#include "word.h"
using words::Word;
using words::itow;
using words::wtoi;
using words::itohw;

Semaphore::Semaphore()
: mCount( itow( 1 ) ) {
}

void Semaphore::Down( Interpreter& b ) {
    if ( wtoi( mCount ) > 0 ) {
        mCount = itow( wtoi( mCount ) - 1 );
        if ( b.Debug() ) {
            b.Trace() << "\n";
        }
    }
    else {
        b->State( Process::BLOCKED_BY_SEMAPHORE );

        mBlocked.push_back( b.mRunningProcess );

        if ( b.Debug() ) {
            b.Trace() << "Process Blocked.\n";
        }
    }
}

void Semaphore::Up( Interpreter& b ) {
    if ( wtoi( mCount ) == 0 ) {
        if ( !mBlocked.empty() ) {
            Process sleeper = mBlocked.front();
            mBlocked.pop_front();

            sleeper.State( Process::READY );
            b.mReadyQueue.push_back( sleeper );

            if ( b.Debug() ) {
                b.Trace() << "Process "
                          << sleeper.ID()
                          << " was woken up.\n";
            }
        }
        else {
            mCount = itow( wtoi( mCount ) + 1 );
            if ( b.Debug() ) {
                b.Trace() << "\n";
            }
        }
    }
}

Semaphore& Semaphore::operator =( const Word& value ) {
    mCount = value;
    return *this;
}