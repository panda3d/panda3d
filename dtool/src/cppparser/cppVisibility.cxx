// Filename: cppVisibility.C
// Created by:  drose (22Oct99)
// 
////////////////////////////////////////////////////////////////////


#include "cppVisibility.h"

ostream &
operator << (ostream &out, CPPVisibility vis) {
  switch (vis) {
  case V_published:
    return out << "__published";

  case V_public:
    return out << "public";

  case V_protected:
    return out << "protected";
 
  case V_private:
    return out << "private";
 
  case V_unknown:
    return out << "unknown";
  }

  return out << "(**invalid visibility**)";
}
