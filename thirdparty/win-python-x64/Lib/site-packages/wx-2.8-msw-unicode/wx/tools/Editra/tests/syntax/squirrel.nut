// Squirrel Syntax Highlighting Test File
// Comments are like this
/** Multiline comments are like this
 * @summary <- Documentation keywords are like this
 */

// Hello World
print("Hello World")

// Literals
local a = 123       // Decimal
local b = 0x0012    // Hexadecimal
local c = 075       // Octal
local d = 'w'       // Char
local e = "string"  // String
local f = "Unclosed string

// Function Definition
function fib(n)
{
    if (n < 2) return 1
    return fib(n-2) + fib(n-1) 
}

// Class Construct
class Foo {
	//constructor
	constructor(a)
	{
		bar = ["bar", 1, 2, 3];
	}

	function PrintBar()
	{
		foreach(i, val in bar)
		{
			::print("idx = " + i + " = " + val + " \n");
		}
	}
}
