// Filename: test_config.C
// Created by:  cary (10Sep98)
// 
////////////////////////////////////////////////////////////////////

#include "dconfig.h"

#define SNARF
Configure(test);

std::string foo = test.GetString("user");
std::string path = test.GetString("LD_LIBRARY_PATH");

ConfigureFn(test)
{
   cout << "AIEE!  Doing work before main()!  The sky is falling!" << endl;
}

main()
{
   cout << "Testing Configuration functionality:" << endl;
   cout << "foo = " << foo << endl;
   cout << "path = " << path << endl;
}
