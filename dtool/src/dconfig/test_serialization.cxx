// Filename: test_serialization.cxx
// Created by:  cary (31Aug98)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "serialization.h"
#include <list>
#include <vector>
#include <algo.h>
#include <string>

typedef std::list<int> intlist;
typedef std::vector<float> floatvec;

intlist refints;
floatvec reffloats;

int ints[10] = { 3, 1, 4, 1, 5, 9, 2, 6, 5, 3 };
float floats[10] = { 0.1, 1.2, 2.3, 3.4, 4.5, 5.6, 6.7, 7.8, 8.9, 9.0 };

void TestToString()
{
   std::string line;
   Serialize::Serializer<intlist> intser(refints);
   Serialize::Serializer<floatvec> floatser(reffloats);

   line = intser;
   cout << "Int list serialization: '" << line << "'" << endl;
   line = floatser;
   cout << "Float vector serialization: '" << line << "'" << endl;
}

void TestFromString()
{
   std::string line;
   Serialize::Serializer<intlist> intser(refints);
   Serialize::Serializer<floatvec> floatser(reffloats);

   line = intser;
   Serialize::Deserializer<intlist> intdes1(line);
   cout << "Int list deserialization: ";
   intlist ides1 = intdes1;
   for (intlist::iterator i=ides1.begin(); i!=ides1.end(); ++i) {
      if (i != ides1.begin())
         cout << ", ";
      cout << (*i);
   }
   cout << endl;
   line = floatser;
   Serialize::Deserializer<floatvec> floatdes1(line);
   cout << "Float vector deserialization: ";
   floatvec fdes1 = floatdes1;
   for (floatvec::iterator j=fdes1.begin(); j!=fdes1.end(); ++j) {
      if (j != fdes1.begin())
         cout << ", ";
      cout << (*j);
   }
   cout << endl;
}

main()
{
   // initialize the collections
   for (int i=0; i<10; ++i) {
      refints.push_back(ints[i]);
      reffloats.push_back(floats[i]);
   }

   // now run tests
   cout << "Serialization to string:" << endl;
   TestToString();
   cout << endl << "Deserialization from string:" << endl;
   TestFromString();
}
