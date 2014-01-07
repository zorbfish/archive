#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "word.h"
#include "process.h"
#include "semaphore.h"
#include "instructions.h"

#include <map>
#include <fstream>
#include <queue>
#include <vector>
#include <string>

namespace brain {
    // Initialization constants for interpreter.
    const bool BRAIN_DEBUG = true;
    const bool NO_DEBUG = false;

    // Tokens declared for use in parsing Brain04 files.
    static const std::string PROGRAM_INDICATOR;
    static const std::string DATA_INDICATOR;
    static const std::string END_INDICATOR;

    class Interpreter {
    public:
        // Force at construction the user to choose
        // whether to run the interpreter in debug mode or not.
        Interpreter( bool debug, int timeslice );
        ~Interpreter();

        // Loads a said brain file into a new process. The
        // routine returns true if the process was loaded
        // successfully and false if otherwise.
        bool Load( const std::string& file );

        // Executes the loaded process until it is halted.
        void Run();

        void SharedMemory( const words::HalfWord& operand, const words::Word& value );
        const words::Word& SharedMemory( const words::HalfWord& operand ) const;

        void Semaphores( const words::HalfWord& operand, const words::Word& value );
        Semaphore& Semaphores( const words::HalfWord& operand );

        words::Word Data();

        bool MessageTo( const words::HalfWord& operand );
        bool MessageFrom( const words::HalfWord& operand );

        std::ofstream& Trace();
        bool Debug() const;

        Process* operator ->();

    private:
        friend class Semaphore;

        bool ContextSwitch();

        // The main data structure that maps the brain file
        // tokens to the specific instruction pointer.
        typedef std::map< const words::HalfWord, brain::instruction > InstructionMap;
        InstructionMap mInstructions;

        // Determines if the recieved opcode is loaded into the instruction map
        // of the interpreter. If it is not the return value 'result' will
        // point to the end of the map instead of the found instruction.
        bool isValidInstruction( const words::HalfWord& opcode,
            InstructionMap::const_iterator& result ) const;

        // Extracts all data from the input brain file and concatenates
        // it into one large string. Used to cache the data with the process
        // instead of carrying around an open file stream until data is loaded.
        std::string ParseData( std::ifstream& );

        // Initializes the mapping of the instruction set to
        // the brain assembly file tokens.
        void InitInstructionSet();

        // The actual processes ready to be executed by the intepreter.
        std::deque< brain::Process > mReadyQueue;
        
        // Message queue for processes sending and recieving messages.
        std::deque< brain::Process > mMessageQueue;

        // Stores the currently running process in the interpreter.
        brain::Process mRunningProcess;

        brain::Process::BrainMemory mSharedMemory;

        Semaphore mSemaphores[ 100 ];

        std::string mCachedData;

        int mTimeSlice;
        int mTimer;
        int mContextSwitchCount;

        // Flag that determines whether or not the
        // intepteter should trace the process' execution.
        bool mDebug;

        // Output files for all debugging output.
        std::ofstream mTraces[ 10 ];

        // Disallowed Functionality.
        Interpreter();
        Interpreter( const Interpreter& );
        Interpreter& operator =( const Interpreter& );
    };

    // Extracts all instructional data from the input brain
    // file and places it into the process' memory.
    bool ParseInstructions( std::ifstream& brainFile, Process& load );
}

#endif
