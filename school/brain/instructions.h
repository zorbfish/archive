#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

// Foward declare the HalfWord structure, and the output stream object.
#include <iosfwd>

namespace words {
    struct HalfWord;
}

namespace brain {
    class Interpreter;

    // Instructions
    // ==================================================================
    // Each instruction recieves:
    //    * A HalfWord, which is the instruction's operand.
    //    * A Process, which is the current process of the instruction
    //    * An optional ostream which is used for tracing purposes.
    //      If not trace was requested by the Interpreter the instruction
    //      should expect this parameter to be a null pointer.

    // Traces
    // ==================================================================
    // All tracing operations are not built for speed. Each tracing block
    // may contain many extraneous calls to the active process to display
    // its state.

    // The Instruction Typedef
    // ==================================================================
    // Defines a function pointer that recieves a halfword, process, and
    // pointer to an output stream. This function pointer is declared as
    // 'instruction'.

    typedef void ( *instruction )( const words::HalfWord&, brain::Interpreter& );

    // Self Explanatory

    void LoadRegister( const words::HalfWord&, brain::Interpreter& );
    void LoadLow( const words::HalfWord&, brain::Interpreter& );
    void LoadHigh( const words::HalfWord&, brain::Interpreter& );

    void SaveRegister( const words::HalfWord&, brain::Interpreter& );

    void ConditionEqual( const words::HalfWord&, brain::Interpreter& );
    void ConditionLessThan( const words::HalfWord&, brain::Interpreter& );

    void BranchTrue( const words::HalfWord&, brain::Interpreter& );
    void BranchUnconditional( const words::HalfWord&, brain::Interpreter& );

    void GetData( const words::HalfWord&, brain::Interpreter& );
    void PutData( const words::HalfWord&, brain::Interpreter& );

    void Add( const words::HalfWord&, brain::Interpreter& );
    void Subtract( const words::HalfWord&, brain::Interpreter& );
    void Multiply( const words::HalfWord&, brain::Interpreter& );
    void Divide( const words::HalfWord&, brain::Interpreter& );

    // NullOp and Halt
    // ===================================================================
    // Although these instructions do not necessarily need the parameters
    // they are declared with, their declarations were made similar to use
    // the mapping scheme for the Interpreter.

    void NullOp( const words::HalfWord&, brain::Interpreter& );

    void Halt( const words::HalfWord&, brain::Interpreter& );

    // Added in Project Part II

    void GetProcessID( const words::HalfWord&, brain::Interpreter& );

    // Message Sending instructions

    void SendMessage( const words::HalfWord&, brain::Interpreter& );
    void RecieveMessage( const words::HalfWord&, brain::Interpreter& );

    // Shared Storage instructions

    void LoadShared( const words::HalfWord&, brain::Interpreter& );
    void SaveShared( const words::HalfWord&, brain::Interpreter& );

    // Semaphore related instructions

    void Down( const words::HalfWord&, brain::Interpreter& );
    void Up( const words::HalfWord&, brain::Interpreter& );
    void SemaphoreInitialize( const words::HalfWord&, brain::Interpreter& );
}

#endif
