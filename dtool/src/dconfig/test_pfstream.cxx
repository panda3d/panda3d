// Filename: test_pfstream.C
// Created by:  cary (31Aug98)
// 
////////////////////////////////////////////////////////////////////

#include <pfstream.h>
#include <string>

void ReadIt(istream& ifs) {
   std::string line;

   while (!ifs.eof()) {
      std::getline(ifs, line);
      if (line.length() != 0)
	 cout << line << endl;
   }
}

main()
{
   IPipeStream ipfs("ls -l");

   ReadIt(ipfs);
}
