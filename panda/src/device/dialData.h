// Filename: dialData.h
// Created by:  jason (07Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef DIAL_DATA
#define DIAL_DATA

#include <pandabase.h>
#include <vector_float.h>

class EXPCL_PANDA DialData {
public:
  INLINE DialData();
  INLINE DialData(const DialData &copy);
  INLINE DialData &operator = (const DialData &copy);

  INLINE static const DialData &none();

  double dtime;
  int dial_id;
  float change;
private:
  static DialData _none;
};

#include "dialData.I"

#endif
