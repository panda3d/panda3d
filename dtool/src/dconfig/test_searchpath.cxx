// Filename: test_searchpath.C
// Created by:  cary (01Sep98)
// 
////////////////////////////////////////////////////////////////////

#include <dSearchPath.h>
//#include <expand.h>
#include <string>

void TestSearch()
{
   std::string line, path;

//   path = ".:~ /etc";
   path = ". /etc";
//   path = Expand::Expand(path);
   line = "searchpath.h";
   cout << "looking for file '" << line << "' in path '" << path << "': '";
   line = DSearchPath::search_path(line, path);
   cout << line << "'" << endl;

   line = ".cshrc";
   cout << "looking for file '" << line << "' in path '" << path << "': '";
   line = DSearchPath::search_path(line, path);
   cout << line << "'" << endl;

   line = "passwd";
   cout << "looking for file '" << line << "' in path '" << path << "': '";
   line = DSearchPath::search_path(line, path);
   cout << line << "'" << endl;
}

main()
{
   cout << "Testing search path:" << endl;
   TestSearch();
}
