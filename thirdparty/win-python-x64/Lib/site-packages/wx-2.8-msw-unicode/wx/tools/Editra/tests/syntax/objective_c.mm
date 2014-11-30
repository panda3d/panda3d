// Syntax Highlighting Test File for Objective C
// Comments are like this
/* Multi line comments are like
 * this
 */

// Hello World
#import <stdio.h>

int main( int argc, const char *argv[] ) {
    printf( "Hello World\n" );
    printf( "Unclosed string );
    return 0;
}

// Interface
#import <Foundation/NSObject.h>

@interface Fraction: NSObject {
    int numerator;
    int denominator;
}

-(void) print;
-(void) setNumerator: (int) d;
-(void) setDenominator: (int) d;
-(int) numerator;
-(int) denominator;
@end

// Class implementation
#import "Fraction.h"
#import <stdio.h>

@implementation Fraction
-(void) print {
    printf( "%i/%i", numerator, denominator );
}

-(void) setNumerator: (int) n {
    numerator = n;
}

-(void) setDenominator: (int) d {
    denominator = d;
}

-(int) denominator {
    return denominator;
}

-(int) numerator {
    return numerator;
}
@end

