#include "process.h"
using std::string;
using namespace brain;
using namespace words;

Process::Process( int id )
: mRegister( "0000" )
, mCounter( "00" )
, mCondition( 'F' )
, mState( READY )
, mID( id ) {
}

Process::Process( const Process& p )
: mRegister( p.mRegister )
, mCounter( p.mCounter )
, mCondition( p.mCondition )
, mState( p.mState )
, mID( p.mID ) {
    for ( int i = 0; i < 100; ++i ) {
        mMemory[ i ] = p.mMemory[ i ];
    }
}

Process& Process::operator =( const Process& rhs ) {
    mRegister = rhs.mRegister;
    mCounter = rhs.mCounter;
    mCondition = rhs.mCondition;
    mState = rhs.mState;
    mID = rhs.mID;

    for ( int i = 0; i < 100; ++i ) {
        mMemory[ i ] = rhs.mMemory[ i ];
    }

    return *this;
}

void Process::Counter( const HalfWord& count ) {
    // Set the counter if count it is a number
    if ( isNumeric( count ) ) {
        mCounter = count;
    }
}

int Process::ID() const {
    return mID;
}

void Process::Memory( const HalfWord& address, const Word& value ) {
    // Set memory if the address is a number
    if ( isNumeric( address ) ) {
        mMemory[ hwtoi( address ) ] = value;
    }
}

Word Process::Memory( const HalfWord& address ) const {
    return mMemory[ hwtoi( address ) ];
}

void Process::Register( const Word& value ) {
    mRegister = value;
}

Word Process::Register() const {
    return mRegister;
}

void Process::Register( bool low, const HalfWord& value ) {
    if ( low ) {
        mRegister.low = value;
    }
    else {
        mRegister.high = value;
    }
}

bool Process::Condition() const {
    return mCondition == 'T';
}

void Process::Condition( char state ) {
    if ( state == 'T' || state == 'F' ) {
        mCondition = state;
    }
}

Process::PROCESS_STATE Process::State() const {
    return mState;
}

void Process::State( Process::PROCESS_STATE state ) {
    mState = state;
}

HalfWord Process::Counter() const {
    return mCounter;
}

void Process::AdvanceCounter() {
    int c = hwtoi( mCounter );
    mCounter = itohw( ++c );
}

bool brain::Halted( const Process& ps ) {
    // Remember: The high byte always contains the opcode for the instruction.
    return 'h' == ps.Memory( ps.Counter() ).high.bytes[ 0 ];
}
