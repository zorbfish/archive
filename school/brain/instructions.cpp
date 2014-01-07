
#include "word.h"
using namespace words;

#include "interpreter.h"
#include "instructions.h"
using namespace brain;

#include <iostream>
using std::string;

void brain::LoadRegister( const HalfWord& operand, Interpreter& b ) {
    b->Register( b->Memory( operand ) );

    if ( b.Debug() ) {
        b.Trace() << "Register loaded with value at address "
                  << operand
                  << ". Register now contains "
                  << b->Register()
                  << "\n";
    }
}

void brain::LoadLow( const HalfWord& operand, Interpreter&  b) {
    // True selects the register's low byte for loading.
    b->Register( true, b->Memory( operand ).low );

    if ( b.Debug() ) {
        b.Trace() << "Register's low byte loaded with bytes at address "
                  << operand
                  << ". Register now contains "
                  << b->Register()
                  << "\n";
    }
}

void brain::LoadHigh( const HalfWord& operand, Interpreter& b ) {
    // False selects the register's high byte for loading.
    b->Register( false, b->Memory( operand ).high );

    if ( b.Debug() ) {
        b.Trace() << "Register's high byte loaded with bytes at address "
                  << operand
                  << ". Register now contains "
                  << b->Register()
                  << "\n";
    }
}

void brain::SaveRegister( const HalfWord& operand, Interpreter& b ) {
    b->Memory( operand, b->Register() );

    if ( b.Debug() ) {
        b.Trace() << "Register's value ["
                  << b->Register()
                  << "] saved into address "
                  << operand
                  << ".\n";
    }
}

void brain::ConditionEqual( const HalfWord& operand, Interpreter& b ) {
    if ( b.Debug() ) {
        b.Trace() << "Is register's value ["
                  << b->Register()
                  << "] equal to address "
                  << operand
                  << "'s value ["
                  << b->Memory( operand )
                  << "]? ";
    }

    if ( b->Register() == b->Memory( operand ) ) {
        b->Condition( 'T' );
        if ( b.Debug() ) {
            b.Trace() << "Yes.\n";
        }
    }
    else {
        b->Condition( 'F' );
        if ( b.Debug() ) {
            b.Trace() << "No.\n";
        }
    }
}

void brain::ConditionLessThan( const HalfWord& operand, Interpreter& b ) {
    if ( b.Debug() ) {
        b.Trace() << "Is register's value ["
                  << b->Register()
                  << "] less than address "
                  << operand
                  << "'s value ["
                  << b->Memory( operand )
                  << "]? ";
    }

    if ( b->Register() < b->Memory( operand ) ) {
        b->Condition( 'T' );
        if ( b.Debug() ) {
            b.Trace() << "Yes.\n";
        }
    }
    else {
        b->Condition( 'F' );
        if ( b.Debug() ) {
            b.Trace() << "No.\n";
        }
    }
}

void brain::BranchTrue( const HalfWord& operand, Interpreter& b ) {
    if ( b.Debug() ) {
        b.Trace() << "Condition register set to True? ";
    }

    if ( b->Condition() ) {
        b->Counter( operand );
        if ( b.Debug() ) {
            b.Trace() << "Yes. Branched to address "
                      << operand
                      << "\n";
        }
    }
    else if ( b.Debug() ) {
        b.Trace() << "No.\n";
    }
}

void brain::BranchUnconditional( const HalfWord& operand, Interpreter& b ) {
    b->Counter( operand );
    if ( b.Debug() ) {
        b.Trace() << "Branched to address "
                  << operand
                  << "\n";
    }
}

void brain::GetData( const HalfWord& operand, Interpreter& b ) {
    // Save the process' counter to restore later.
    HalfWord save = b->Counter();

    // To make iterating the memory map of the process easier. I reset the
    // instruction counter to point to the area where the data is to be loaded.

    // Compute the new address to point the counter at.
    int loc = hwtoi( operand );
    loc -= ( loc % 10 );
    b->Counter( itohw( loc ) );
    for ( int j = 0; j < 10; ++j ) {
        // Fetch the next word from the cached data string and place it into memory.
        b->Memory( b->Counter(), b.Data() );
        b->AdvanceCounter();
    }
    
    // Restore the instruction counter.
    b->Counter( save );

    if ( b.Debug() ) {
        b.Trace() << "Data was retrieved from file and placed at address "
                  << operand
                  << ".\n";
    }
}

void brain::PutData( const HalfWord& operand, Interpreter& b ) {
    // Save the process' counter to restore later.
    HalfWord save = b->Counter();

    // To make iterating the memory map of the process easier. I reset the
    // instruction counter to point to the area where the data is to be loaded.

    // Compute the new address to point the counter at.
    int loc = hwtoi( operand );
    loc -= ( loc % 10 );
    b->Counter( itohw( loc ) );
    for ( int j = 0; j < 10; ++j ) {
        // Fetch the next word to print to the console.
        Word out = b->Memory( b->Counter() );

        // If the word is numeric remove any leading zeroes.
        // This was just a preference, it has no effect on
        // the design of the Brain.
        if ( isNumeric( out ) ) {
            std::cout << wtoi( out )
                      << " ";
        }
        else {
            // Else print the word.
            std::cout << out;
        }
        b->AdvanceCounter();
    }
    
    // End the line after printing 10 data items.
    std::cout << std::endl;
    b->Counter( save );

    if ( b.Debug() ) {
        b.Trace() << "Data at address "
                  << operand
                  << " was written to the console.\n";
    }
}

void brain::Add( const HalfWord& operand, Interpreter& b ) {
    Word reg = b->Register();
    Word val = b->Memory( operand );
    // Make sure both operands are numeric before attempting to add them.
    if ( isNumeric( reg ) && isNumeric( val ) ) {
        // Perform the addition and save it to the register.
        int r = wtoi( reg ) + wtoi( val );
        b->Register( itow( r ) );
    }
    else {
        // Throw an exception if the instruction tried to operate on non-numeric data.
        throw std::runtime_error( "Arithmetic on non-numeric data" );
    }

    if ( b.Debug() ) {
        b.Trace() << wtoi( b->Memory( operand ) )
                  << " was added to Register's value. Register now contains "
                  << b->Register()
                  << "\n";
    }
}

void brain::Subtract( const HalfWord& operand, Interpreter& b ) {
    Word reg = b->Register();
    Word val = b->Memory( operand );
    // Make sure both operands are numeric before attempting to subtract them.
    if ( isNumeric( reg ) && isNumeric( val ) ) {
        // Perform the subtraction and check the result.
        int r = wtoi( reg ) - wtoi( val );
        if ( r < 0 ) {
            // Throw an exception for computing a negative result.
            throw std::runtime_error( "Negative difference" );
        }
        
        // Else save it to the register.
        b->Register( itow( r ) );
    }
    else {
        // Throw an exception if the instruction tried to operate on non-numeric data.
        throw std::runtime_error( "Arithmetic on non-numeric data" );
    }

    if ( b.Debug() ) {
        b.Trace() << wtoi( b->Memory( operand ) )
                  << " was subtracted to Register's value. Register now contains "
                  << b->Register()
                  << "\n";
    }
}

void brain::Multiply( const HalfWord& operand, Interpreter& b ) {
    Word reg = b->Register();
    Word val = b->Memory( operand );
    // Make sure both operands are numeric before attempting to multiply them.
    if ( isNumeric( reg ) && isNumeric( val ) ) {
        // Perform the multiplication and store the result in the register.
        int r = wtoi( reg ) * wtoi( val );
        b->Register( itow( r ) );
        
    }
    else {
        // Throw an exception if the instruction tried to operate on non-numeric data.
        throw std::runtime_error( "Arithmetic on non-numeric data" );
    }

    if ( b.Debug() ) {
        b.Trace() << wtoi( val )
                  << " was multiplied by Register's value. Register now contains "
                  << b->Register()
                  << "\n";
    }
}

void brain::Divide( const HalfWord& operand, Interpreter& b ) {
    Word reg = b->Register();
    Word val = b->Memory( operand );
    // Make sure both operands are numeric before attempting to divide them.
    if ( isNumeric( reg ) && isNumeric( val ) ) {
        // Perform the division and check the result.
        int r = wtoi( reg );
        int div = wtoi( val );
        if ( div == 0 ) {
            // Throw an exception for trying to divide by zero.
            throw std::runtime_error( "Divide By Zero" );
        }
        
        // Else set the result to the register.
        b->Register( itow( r / div ) );
        
    }
    else {
        // Throw an exception if the instruction tried to operate on non-numeric data.
        throw std::runtime_error( "Arithmetic on non-numeric data" );
    }

    if ( b.Debug() ) {
        b.Trace() << "Register's value was divided by "
                  << wtoi( val )
                  << ". Register now contains "
                  << b->Register()
                  << "\n";
    }
}

void brain::NullOp( const HalfWord&, Interpreter& b ) {
    // No operation performed here.
    if ( b.Debug() ) {
        b.Trace() << "No operation executed.\n";
    }
}

void brain::Halt( const HalfWord&, Interpreter& b ) {
    b->State( Process::EXECUTION_COMPLETE );
    if ( b.Debug() ) {
        b.Trace() << "Execution finished.\n";
    }
}

void brain::GetProcessID( const HalfWord&, Interpreter& b ) {
    b->Register( itow( b->ID() ) );

    if ( b.Debug() ) {
        b.Trace() << "Register was loaded with"
                  << " the processes' ID [ "
                  << itow( b->ID() )
                  << " ].\n";
    }
}

void brain::LoadShared( const HalfWord& operand, Interpreter& b ) {
    b->Register( b.SharedMemory( operand ) );

    if ( b.Debug() ) {
        b.Trace() << "Register loaded with value at shared address "
                  << operand
                  << ". Register now contains "
                  << b->Register()
                  << "\n";
    }
}

void brain::SaveShared( const HalfWord& operand, Interpreter& b ) {
    b.SharedMemory( operand, b->Register() );

    if ( b.Debug() ) {
        b.Trace() << "Register's value ["
                  << b->Register()
                  << "] saved into shared address "
                  << operand
                  << ".\n";
    }
}

void brain::SendMessage( const HalfWord& operand, Interpreter& b ) {
    if ( b.Debug() ) {
        b.Trace() << "Attempting to send message at address "
                  << wtoi( b->Register() )
                  << " to process "
                  << hwtoi( operand )
                  << ".\n";
    }

    if ( b.MessageTo( operand ) ) {
        if ( b.Debug() ) {
            b.Trace() << "The message was sent.\n";
        }
    }
    else {
        if ( b.Debug() ) {
            b.Trace() << "Reciever not ready, the sender is now blocked.\n";
        }
    }
}

void brain::RecieveMessage( const HalfWord& operand, Interpreter& b ) {
    if ( b.Debug() ) {
        b.Trace() << "Attempting to retrieve message from ";

        if ( hwtoi( operand ) == 792 ) {
            b.Trace() << "any process";
        }
        else {
            b.Trace() << "process " << hwtoi( operand );
        }
        
        b.Trace() << ".\n";
    }

    if ( b.MessageFrom( operand ) ) {
        if ( b.Debug() ) {
            b.Trace() << "Message recieved and stored at address "
                      << b->Register().low
                      << ".\n";
        }
    }
    else {
        if ( b.Debug() ) {
            b.Trace() << "Message not sent, the reciever is now blocked.\n";
        }
    }
}

void brain::Down( const HalfWord& operand, Interpreter& b ) {
    if ( b.Debug() ) {
        b.Trace() << "Down on semaphore "
                  << hwtoi( operand )
                  << ". ";
    }

    b.Semaphores( operand ).Down( b );
}

void brain::Up( const HalfWord& operand, Interpreter& b ) {
    if ( b.Debug() ) {
        b.Trace() << "Up on semaphore "
                  << hwtoi( operand )
                  << ". ";
    }

    b.Semaphores( operand ).Up( b );
}

void brain::SemaphoreInitialize( const HalfWord& operand, Interpreter &b ) {
    b.Semaphores( operand, b->Register() );

    if ( b.Debug() ) {
        b.Trace() << "Semaphore "
                  << hwtoi( operand )
                  << " was initialized to value in Register ["
                  << b->Register()
                  << "].\n";
    }
}
