// Filename: test_expand.C
// Created by:  cary (31Aug98)
// 
////////////////////////////////////////////////////////////////////

#include "expand.h"
#include <string>

void TestExpandFunction()
{
   std::string line;

   line = "foo";
   cout << "input: '" << line << "'" << endl;
   cout << "output: '" << Expand::Expand(line) << "'" << endl;
   line = "'foo'";
   cout << "input: '" << line << "'" << endl;
   cout << "output: '" << Expand::Expand(line) << "'" << endl;
   line = "'$USER'";
   cout << "input: '" << line << "'" << endl;
   cout << "output: '" << Expand::Expand(line) << "'" << endl;
   line = "$USER";
   cout << "input: '" << line << "'" << endl;
   cout << "output: '" << Expand::Expand(line) << "'" << endl;
   line = "\"$USER\"";
   cout << "input: '" << line << "'" << endl;
   cout << "output: '" << Expand::Expand(line) << "'" << endl;
   line = "`ls -l`";
   cout << "input: '" << line << "'" << endl;
   cout << "output: '" << Expand::Expand(line) << "'" << endl;
   line = "~";
   cout << "input: '" << line << "'" << endl;
   cout << "output: '" << Expand::Expand(line) << "'" << endl;
   line = "~cary";
   cout << "input: '" << line << "'" << endl;
   cout << "output: '" << Expand::Expand(line) << "'" << endl;
}

void TestExpandClass()
{
   std::string line;

   line = "foo";
   Expand::Expander ex(line);
   cout << "input: '" << line << "'" << endl;
   cout << "output: '" << ex() << "'" << endl;
   line = "'foo'";
   cout << "input: '" << line << "'" << endl;
   cout << "output: '" << ex(line) << "'" << endl;
   line = "'$USER'";
   cout << "input: '" << line << "'" << endl;
   cout << "output: '" << ex(line) << "'" << endl;
   line = "$USER";
   cout << "input: '" << line << "'" << endl;
   cout << "output: '" << ex(line) << "'" << endl;
   line = "\"$USER\"";
   cout << "input: '" << line << "'" << endl;
   cout << "output: '" << ex(line) << "'" << endl;
   line = "`ls -l`";
   cout << "input: '" << line << "'" << endl;
   cout << "output: '" << ex(line) << "'" << endl;
   line = "~";
   cout << "input: '" << line << "'" << endl;
   cout << "output: '" << ex(line) << "'" << endl;
   line = "~cary";
   cout << "input: '" << line << "'" << endl;
   cout << "output: '" << ex(line) << "'" << endl;
}

main()
{
   cout << endl << "Testing shell expansion (function version):" << endl;
   TestExpandFunction();
   cout << endl << "Testing shell expansion (class version):" << endl;
   TestExpandClass();
}
