
#include "interpreter.h"
using namespace brain;

#include <iostream>
using std::cout;

// Brain Entry Point
int main( int argc, char *argv[] ) {
    if ( argc < 2 ) {
        // Output an error if the user did not specify
        // enough parameters to execute brain correctly.
        cout << "Brain expects at least 1 parameter: the file to execute.\n";
        return 1;
    }

    if ( argc == 2 ) {
        // Run brain with no debugging, and default time slices
        Interpreter b( brain::NO_DEBUG, 5 );

        if ( b.Load( argv[ 1 ] ) ) {
            b.Run();
        }
    }

    if ( argc == 3 ) {
        // Run brain with no debugging.
        Interpreter b( brain::NO_DEBUG, atoi( argv[ 2 ] ) );

        if ( b.Load( argv[ 1 ] ) ) {
            b.Run();
        }
    }
    // If the user sent the d character as the last parameter, run Brain in debug mode.
    else if ( argc == 4 && *argv[ 3 ] == 'd' || *argv[ 3 ] == 'D' ) {
        Interpreter b( brain::BRAIN_DEBUG, atoi( argv[ 2 ] ) );

        if ( b.Load( argv[ 1 ] ) ) {
            b.Run();
        }
    }

    return 0;
}