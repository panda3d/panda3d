// Filename: test_serialization.C
// Created by:  cary (31Aug98)
// 
////////////////////////////////////////////////////////////////////

#include <serialization.h>
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
