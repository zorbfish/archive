#ifndef PROCESS_H
#define PROCESS_H

#include <string>
#include "word.h"

namespace brain {
    class Process {
    public:
        Process( const Process& p );
        Process& operator =( const Process& rhs );

        // The actual memory model of the Process.
        typedef words::Word BrainMemory[ 100 ];

        enum PROCESS_STATE {
            READY = 0
            , BLOCKED_BY_SEMAPHORE
            , BLOCKED_BY_SEND
            , BLOCKED_BY_RECIEVE
            , EXECUTION_COMPLETE
        };

        // Initialize the process with its 
        // identifier at construction time.
        explicit Process( int id );

        // Retrieve the id of the process.
        int ID() const;

        // Retrieves the instruction counter value.
        words::HalfWord Counter() const;

        // Sets the instruction counter to a new address.
        void Counter( const words::HalfWord& a );

        // Advances the instruction counter by 1.
        void AdvanceCounter();

        // Retrieves the word value at a specific memory location.
        words::Word Memory( const words::HalfWord& ) const;

        // Sets the word value at a specific memory location.
        void Memory( const words::HalfWord&, const words::Word& );

        // Sets the value of the register word.
        void Register( const words::Word& );

        // Allows the register's high and low half word to be set.
        void Register( bool low, const words::HalfWord& );

        // Retrieves the value of the register.
        words::Word Register() const;

        // Retrieves the resulting expression from the condition code register.
        // I.E. Internally performs the explicit: condition_ == 'T' expression.
        bool Condition() const;

        // Sets the condition code register to either 'T' or 'F'.
        void Condition( char state );

        // Retrieves the current state of the Process.
        PROCESS_STATE State() const;
        void State( PROCESS_STATE state );

    private:
        BrainMemory mMemory;
        words::HalfWord mCounter;
        char mCondition;
        words::Word mRegister;

        // Determines the state of the Process.
        PROCESS_STATE mState;

        // Identifier of the Process.
        int mID;
    };

    // Predicate the returns true if the next instruction to be executed is a halt instruction.
    bool Halted( const Process& ps );
}

#endif
