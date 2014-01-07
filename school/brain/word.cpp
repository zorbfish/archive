#include "word.h"
using namespace words;

#include <iostream>
using std::ostream;

// Word and HalfWord functionality
// ==================================================================
// The majority of the functionality is only built for half words.
// Because a word is comprised of two half words all the previous
// functionality was reused to build the functionality of words.

//
// HALFWORD MEMBERS
//

HalfWord::HalfWord() {
    bytes[ 0 ]
    = bytes[ 1 ] = '0';
}

HalfWord::HalfWord( const char *b ) {
    bytes[ 0 ] = b[ 0 ];
    bytes[ 1 ] = b[ 1 ];
}

HalfWord &HalfWord::operator =( const HalfWord &rhs ) {
    bytes[ 0 ] = rhs.bytes[ 0 ];
    bytes[ 1 ] = rhs.bytes[ 1 ];
    return *this;
}

//
// WORD MEMBERS
//

Word::Word() {
    high.bytes[ 0 ]
    = high.bytes[ 1 ]
    = low.bytes[ 0 ]
    = low.bytes[ 1 ] = '0';
}

Word::Word( const char *b ) {
    high.bytes[ 0 ] = b[ 0 ];
    high.bytes[ 1 ] = b[ 1 ];
    low.bytes[ 0 ]  = b[ 2 ];
    low.bytes[ 1 ]  = b[ 3 ];
}

Word &Word::operator =( const Word &rhs ) {
    high = rhs.high;
    low = rhs.low;
    return *this;
}

//
// CONVERSION METHODS
//

HalfWord words::itohw( int i ) {
    // Make sure we do not overflow
    // the half word we are trying to create.
    if ( i > 99 ) {
        // This will strip off all but the last 2 digits.
        i = i  % 100;
    }

    HalfWord h;

    int r = i / 10;
    i -= r * 10;
    h.bytes[ 0 ] = r + '0';

    h.bytes[ 1 ] = i + '0';
    
    return h;
}

int words::hwtoi( const HalfWord &hw ) {
    return ( hw.bytes[ 0 ] - '0' ) * 10 + ( hw.bytes[ 1 ] - '0' );
}

Word words::itow( int i ) {
    // Make sure we do not do not overflow
    // the word we are trying to create.
    if ( i > 9999 ) {
        // This will strip off all but the last 4 digits.
        i = i % 10000;
    }

    Word w;
    w.high = itohw( i / 100 );
    w.low = itohw( i );

    return w;
}

int words::wtoi( const Word &w ) {
    return hwtoi( w.high ) * 100 + hwtoi( w.low );
}

//
// NUMERIC PREDICATES
//

bool words::isNumeric( const HalfWord &h ) {
    return h.bytes[ 0 ] >= '0' && h.bytes[ 0 ] <= '9'
        && h.bytes[ 1 ] >= '0' && h.bytes[ 1 ] <= '9';
}

bool words::isNumeric( const Word & w ) {
    return isNumeric( w.high ) && isNumeric( w.low );
}

//
// OUTPUT OPERATORS
//

ostream &words::operator <<( ostream &out, const Word &w ) {
    out << w.high.bytes[ 0 ] << w.high.bytes[ 1 ];
    out << w.low.bytes[ 0 ] << w.low.bytes[ 1 ];
    return out;
}

ostream &words::operator <<( ostream &out, const HalfWord &w ) {
    out << w.bytes[ 0 ] << w.bytes[ 1 ];
    return out;
}

//
// RELATIONAL OPERATORS
//

bool words::operator <( const HalfWord &lhs, const HalfWord &rhs ) {
    return hwtoi( lhs ) < hwtoi( rhs );
}

bool words::operator ==( const HalfWord &lhs, const HalfWord &rhs ) {
    return hwtoi( lhs ) == hwtoi( rhs );
}

bool words::operator <( const Word &lhs, const Word &rhs ) {
    return wtoi( lhs ) < wtoi( rhs );
}

bool words::operator ==( const Word &lhs, const Word &rhs ) {
    return wtoi( lhs ) == wtoi( rhs );
}
