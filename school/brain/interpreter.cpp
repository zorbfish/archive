#include "interpreter.h"
using namespace brain;
using namespace words;

#include <iostream>
#include <sstream>
using std::stringstream;

#include <string>
using std::string;

#include <utility>
using std::make_pair;

#include <algorithm>
using std::transform;

// Initialize parsing tokens for the interpreter.
const string PROGRAM_INDICATOR = "brain";
const string DATA_INDICATOR = "data";
const string END_INDICATOR = "end";

// This routine extracts the operator code from an line fed
// to it as input and returns the operator code as a string.
// The returned string is always less than or equal to two
// characters in length.
Word ExtractInstruction( const string &line ) {
    // Look for the first space character on the input line
    // and use this to split the remainder of the line from
    // the the instruction.
    string ins = line.substr( 0, line.find_first_of( " " ) );
    
    // Resize the new string if it is still longer than 4 characters.
    if ( ins.length() > 4 ) {
        ins.resize( 4 );
    }

    // Convert remaining string to lower case for string comparisons.
    transform( ins.begin(), ins.end(), ins.begin(), tolower );

    Word w;
    w.high.bytes[ 0 ] = ins[ 0 ];
    w.high.bytes[ 1 ] = ins[ 1 ];
    w.low.bytes[ 0 ]  = ins[ 2 ];
    w.low.bytes[ 1 ]  = ins[ 3 ];
    return w;
}

Interpreter::Interpreter( bool debug, int timeslice )
: mDebug( debug )
, mTimeSlice( timeslice )
, mContextSwitchCount( 0 )
, mTimer( 0 )
, mRunningProcess( -1 ) {
    // Initialize the interpretor with the instruction set at construction
    InitInstructionSet();
}

Interpreter::~Interpreter() {
    if ( mDebug ) {
        for ( int i = 0; i < 10; ++i ) {
            if ( mTraces[ i ].is_open() ) {
                mTraces[ i ].close();
            }
        }
    }
}

bool Interpreter::Debug() const {
    return mDebug;
}

Process* Interpreter::operator ->() {
    return &mRunningProcess;
}

std::ofstream& Interpreter::Trace() {
    return mTraces[ ( *this )->ID() ];
}

bool Interpreter::Load( const string& file ) {
    // Check if the file was opened successfully.
    std::ifstream input( file.c_str() );
    if ( input.is_open() ) {
        string line;

        // If it was opened, read in the first line of the
        // file and check if its the program header. Report
        // and error if the program did not contain a header.
        getline( input, line );
        transform( line.begin(), line.end(), line.begin(), tolower );
        if ( !input.eof() && line.find( PROGRAM_INDICATOR ) == -1 ) {
            // Report a malformed brain file was loaded.
            std::cout << "Brain could not locate the first program unit header in "
                      << file
                      << ". File cannot be executed.\n";
        }
        else {
            // Load the ten processes into memory
            int MAX_PROCESSES = 10;
            int processesLeft = 0;
            bool loadOK = true;
            while ( processesLeft < MAX_PROCESSES && loadOK ) {
                // Construct the next process to load from file.
                Process nextProcess( processesLeft );

                // Extract the instructions from file and place them in the process.
                // Also determine if there are more processes to load.
                try {
                    loadOK = ParseInstructions( input, nextProcess );
                }

                // This should catch any loading exceptions and report the error
                // to the console. Also it returns false indicating the brain's
                // Load method failed to complete execution.
                catch ( std::runtime_error& rt ) {
                    std::cerr << rt.what() << std::endl;
                    return false;
                }

                // We perform a check just incase the parse instructions call
                // put nothing in the process so that we do not include an
                // empty process object onto the ready queue.
                if ( loadOK ) {
                    mReadyQueue.push_back( nextProcess );
                    stringstream s;
                    s << "process" << processesLeft << ".txt" << std::ends;
                    mTraces[ processesLeft ].open( s.str().c_str() );
                }

                ++processesLeft;
            }

            // Load data here
            mCachedData = ParseData( input );

            // Close the input file stream.
            input.close();

            mRunningProcess = mReadyQueue.front();
            mReadyQueue.pop_front();

        }
    }
    else {
        // Else, report that the selected file did not open correctly.
        std::cout << file
                  << " could not be opened.\n";
    }

    // The file was successfully loaded.
    return true;
}

void Interpreter::SharedMemory( const HalfWord& operand, const Word& value ) {
    mSharedMemory[ hwtoi( operand ) ] = value;
}

const Word& Interpreter::SharedMemory( const HalfWord& operand ) const {
    return mSharedMemory[ hwtoi( operand ) ];
}

bool Interpreter::ContextSwitch() {
    if ( mRunningProcess.State() == Process::READY ) {
        mReadyQueue.push_back( mRunningProcess );
    }

    if ( !mReadyQueue.empty() ) {
        if ( Debug() ) {
            Trace() << "Context Switch: Process "
                    << mRunningProcess.ID()
                    << " to Process "
                    << mReadyQueue.front().ID()
                    << ".\n";
        }

        mRunningProcess = mReadyQueue.front();
        mReadyQueue.pop_front();
    
        mTimer = 0;
        ++mContextSwitchCount;

        return true;
    }
    else {
        if ( !mMessageQueue.empty() ) {
            std::cerr << "System has deadlocked." << std::endl;
        }
    }

    return false;
}

void Interpreter::Semaphores( const HalfWord& operand, const Word& value ) {
    mSemaphores[ hwtoi( operand ) ] = value;
}

Semaphore& Interpreter::Semaphores( const HalfWord& operand ) {
    return mSemaphores[ hwtoi( operand ) ];
}

Word Interpreter::Data() {
    Word w;
    // Extract the next 4 characters off
    // the cached data line.
    if ( mCachedData.length() >= 4 ) {
        string word = mCachedData.substr( 0, 4 );
        // Strip off those four characters.
        mCachedData = mCachedData.substr( 4 );

        // Convert this string to a word and return it.
        w.high.bytes[ 0 ] = word[ 0 ];
        w.high.bytes[ 1 ] = word[ 1 ];
        w.low.bytes[ 0 ] = word[ 2 ];
        w.low.bytes[ 1 ] = word[ 3 ];
    }
    return w;
}

void Interpreter::Run() {
    // Continue executing instructions as long as
    // a halt instruction is not encountered.
    while ( mTimer < mTimeSlice && mRunningProcess.State() != Process::EXECUTION_COMPLETE 
        && mRunningProcess.State() == Process::READY ) {
        // Fetch the next instruction in memory and
        // advance the instruction to the next word.
        Word ins = mRunningProcess.Memory( mRunningProcess.Counter() );
        mRunningProcess.AdvanceCounter();

        // If the instruction was found, execute it.
        InstructionMap::iterator found;
        if ( isValidInstruction( ins.high, found ) ) {
            try {
                ( *found->second )( ins.low, *this );
                ++mTimer;
            }

            catch( std::runtime_error &rt ) {
                // If we caught an error report it to the
                // console, close the trace file if open,
                // and abort the remaining execution.
                std::cout << "Error: "
                          << rt.what()
                          << " on line "
                          << hwtoi( mRunningProcess.Counter() )
                          << "\n";

                // Close the trace file if available.
                
                break;
            }
        }
    }

    if ( ContextSwitch() ) {
        Run();
    }
}

bool brain::ParseInstructions( std::ifstream& input, Process& load ) {
    static string line;
    
    if ( input.eof() || line.substr( 0, 4 ) == "data" ) {
        return false;
    }

    getline( input, line );

    // Extract instructions from the file until
    // we either hit the end of the file or we
    // reach the data indicator.
    int linesRead = 1;

    while ( !input.eof() && line.substr( 0, 4 ) != "data"
        && line.substr( 0, 5 ) != "brain" ) {

        if ( line.length() > 0 ) {
            // Extract the instruction from the line we just read into memory.
            Word ins = ExtractInstruction( line );

            load.Memory( load.Counter(), ins );
            load.AdvanceCounter();
      
           ++linesRead;

           // Make sure the lines read into the brain's memory
           // have not exceeded the 99 word limit.
           if ( linesRead > 99 ) {
               throw std::runtime_error( "Error: Program has overflowed Brain's memory limit.\n" );
           }
        }

        // Read in the next line from the file and increment the line counter.
        getline( input, line );
    }

   load.Counter( "00" );

   // Process was loaded successfully.
   return true;
}

string Interpreter::ParseData( std::ifstream &input ) {
    string line;
    getline( input, line );

    // Data extracted from the file is stored in the data string.
    string data;

    // Continue extracting the first 40 characters of each line
    // obtained from the file until we either reach the end of file
    // or the end of program indicator.
    while ( !input.eof() && line.substr( 0, 3 ) != "end" ) {
        // Append the next 40 characters to the data string.
        data += line.substr( 0, 40 );
        getline( input, line );
    }

    // Return the collected data.
    return data;
}

bool Interpreter::MessageTo( const HalfWord& operand ) {
    std::deque< Process >::iterator i = mMessageQueue.begin();
    std::deque< Process >::iterator end = mMessageQueue.end();

    for ( ; i != end; ++i ) {
        if ( i->State() == Process::BLOCKED_BY_RECIEVE ) {
            HalfWord h = i->Memory( itohw( hwtoi( i->Counter() ) - 1 ) ).low;
            if ( i->ID() == hwtoi( operand ) || h == HalfWord( "xx" ) ) {
                Process& p = mRunningProcess;

                HalfWord senderSave = p.Counter();
                HalfWord recieverSave = i->Counter();

                p.Counter( p.Register().low );
                i->Counter( i->Register().low );

                for ( int c = 0; c < 10; ++c ) {
                    i->Memory( i->Counter(), p.Memory( p.Counter() ) );

                    i->AdvanceCounter();
                    p.AdvanceCounter();
                }

                p.Counter( senderSave );
                i->Counter( recieverSave );

                i->State( Process::READY );
                mReadyQueue.push_back( *i );

                mMessageQueue.erase( i );

                return true;
            }
        }
    }

    mRunningProcess.State( Process::BLOCKED_BY_SEND );
    mMessageQueue.push_back( mRunningProcess );

    return false;
}

bool Interpreter::MessageFrom( const HalfWord& operand ) {
    std::deque< Process >::iterator i = mMessageQueue.begin();
    std::deque< Process >::iterator end = mMessageQueue.end();
    for ( ; i != end; ++i ) {
        if ( i->State() == Process::BLOCKED_BY_SEND ) {
            if ( mRunningProcess.ID() == hwtoi( i->Memory( itohw( hwtoi( i->Counter() ) - 1 ) ).low ) ) {
                Process& p = mRunningProcess;

                HalfWord recieverSave = p.Counter();
                HalfWord senderSave = i->Counter();

                p.Counter( p.Register().low );
                i->Counter( i->Register().low );

                for ( int c = 0; c < 10; ++c ) {
                    p.Memory( p.Counter(), i->Memory( i->Counter() ) );

                    i->AdvanceCounter();
                    p.AdvanceCounter();
                }

                p.Counter( recieverSave );
                i->Counter( senderSave );

                i->State( Process::READY );
                mReadyQueue.push_back( *i );

                mMessageQueue.erase( i );

                return true;
            }
        }
    }

    mRunningProcess.State( Process::BLOCKED_BY_RECIEVE );
    mMessageQueue.push_back( mRunningProcess );

    return false;
}

bool Interpreter::isValidInstruction( const HalfWord& opcode,
    InstructionMap::const_iterator& result ) const {
    result = mInstructions.find( opcode );
    return result != mInstructions.end();
}

void Interpreter::InitInstructionSet() {
    mInstructions.insert( make_pair( "lr", LoadRegister ) );
    mInstructions.insert( make_pair( "ll", LoadLow ) );
    mInstructions.insert( make_pair( "lh", LoadHigh ) );
    mInstructions.insert( make_pair( "sr", SaveRegister ) );
    mInstructions.insert( make_pair( "ce", ConditionEqual ) );
    mInstructions.insert( make_pair( "cl", ConditionLessThan ) );
    mInstructions.insert( make_pair( "bt", BranchTrue ) );
    mInstructions.insert( make_pair( "bu", BranchUnconditional ) );
    mInstructions.insert( make_pair( "gd", GetData ) );
    mInstructions.insert( make_pair( "pd", PutData ) );
    mInstructions.insert( make_pair( "ad", Add ) );
    mInstructions.insert( make_pair( "su", Subtract ) );
    mInstructions.insert( make_pair( "mu", Multiply ) );
    mInstructions.insert( make_pair( "di", Divide ) );
    mInstructions.insert( make_pair( "np", NullOp ) );
    mInstructions.insert( make_pair( "h", Halt ) );
    mInstructions.insert( make_pair( "sd", SendMessage ) );
    mInstructions.insert( make_pair( "rc", RecieveMessage ) );
    mInstructions.insert( make_pair( "gp", GetProcessID ) );
    mInstructions.insert( make_pair( "ls", LoadShared ) );
    mInstructions.insert( make_pair( "ss", SaveShared ) );
    mInstructions.insert( make_pair( "pe", Down ) );
    mInstructions.insert( make_pair( "ve", Up ) );
    mInstructions.insert( make_pair( "si", SemaphoreInitialize ) );
}
