// Filename: omitReason.cxx
// Created by:  drose (02Dec00)
// 
////////////////////////////////////////////////////////////////////

#include "omitReason.h"

ostream &
operator << (ostream &out, OmitReason omit) {
  switch (omit) {
  case OR_none:
    return out << "none";

  case OR_working:
    return out << "working";

  case OR_omitted:
    return out << "omitted";
   
  case OR_size:
    return out << "size";

  case OR_solitary:
    return out << "solitary";

  case OR_coverage:
    return out << "coverage";

  case OR_unknown:
    return out << "unknown";
  }

  return out << "**invalid**(" << (int)omit << ")";
}
