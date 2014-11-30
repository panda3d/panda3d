// Syntax Highlighting test file for Pike
/* Some Comments about this file */

// Hello world in Pike
int main() {
    write("Hello world!\n");
    return 0;
}

//! \todo <- Documentation Doxygen Keyword highlighting
mixed something;
something = 5;
something = 2.5;

string testString;
testString = "a regular string";
testString = "an open string

// Print a list of unique characters
string data = "an apple a day";
array(string) chars = data/"";
mapping(string:int) seen = ([]);

foreach(chars ;; string char)
  seen[char]++; 

write("unique chars are: %s\n", sort(indices(seen))*"");

