# Syntax Highlighting Test File for Boo
# Comment line
/* This is also a comment line but highlighting is not
*  currently supported by Editra
*/
// So is this but also not currently supported

import System
name = Console.ReadLine()
print "Hello, ${name}"

# Class Def
class TestClass:
    public Name as string
    public Size as int

# More complex class def
public abstract class Hello:
    abstract def Hello():
        print "HELLO"

# Function Def
def fib():
    a, b = 0L, 1L
    while true:
        yield b
        a, b = b, a + b
