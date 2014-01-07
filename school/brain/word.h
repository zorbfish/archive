#ifndef WORD_H
#define WORD_H

#include <iosfwd>

namespace words {
    // Defines a 2 character structure used in
    // modeling the Brain04 virtual machine's memory.
    struct HalfWord {
        // Constructs the characters each
        // with the default value of '0'.
        HalfWord();

        // Constructs a halfword from char pointer
        // that points to 2 characters.
        HalfWord( const char * );

        // Reassigns the characters
        HalfWord &operator =( const HalfWord & );
        
        char bytes[ 2 ];
    };

    // Defines a 4 character structure used in
    // modeling the Brain04 virtual machine's memory.
    struct Word {
        // Constructs the characters each
        // with the default value of '0'.
        Word();

        // Constructs a halfword from char pointer
        // that points to 4 characters.
        Word( const char * );

        // Reassigns the halfwords.
        Word &operator =( const Word & );

        HalfWord high;
        HalfWord low;
    };

    // These two routines convert halfwords to and from integers.
    HalfWord itohw( int );
    int hwtoi( const HalfWord & );
    
    // These two routines convert words to and from integers.
    Word itow( int );
    int wtoi( const Word & );

    // These routines test each character to determine if
    // each one is in the range of '0' <= r <= '9'.
    bool isNumeric( const HalfWord & );
    bool isNumeric( const Word & );

    // Outputs the character composition of halfwords/words to output streams.
    std::ostream &operator <<( std::ostream &, const HalfWord & );
    std::ostream &operator <<( std::ostream &, const Word & );
    
    // Relational Operators. The majority of these were
    // only needed for use with the standard template library.
    bool operator < ( const HalfWord &, const HalfWord & );
    bool operator ==( const HalfWord &, const HalfWord & );

    bool operator < ( const Word &, const Word & );
    bool operator ==( const Word &, const Word & );
}

#endif
