// Filename: cppVisibility.h
// Created by:  drose (22Oct99)
//
////////////////////////////////////////////////////////////////////

#ifndef CPPVISIBILITY_H
#define CPPVISIBILITY_H

#include <dtoolbase.h>

enum CPPVisibility {
  V_published,
  V_public,
  V_protected,
  V_private,
  V_unknown
};

ostream &operator << (ostream &out, CPPVisibility vis);

#endif

