// Filename: analogData.h
// Created by:  jason (04Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef ANALOG_DATA
#define ANALOG_DATA

#include <pandabase.h>
#include <vector_double.h>

class EXPCL_PANDA AnalogData {
public:
  INLINE AnalogData();
  INLINE ~AnalogData();
  INLINE AnalogData(const AnalogData &copy);
  INLINE AnalogData &operator = (const AnalogData &copy);

  INLINE static const AnalogData &none();

  double atime;
  //These two pointers are so that we can return a reference to the
  //current channel values, but not incure the overhead of copying all
  //those values everywhere.  We need two so that we can implement a
  //rotating pointer scheme to ensure that there are no collisions
  //between writing and reading.
  vector_double *channels;  
private:
  vector_double *stored_channels;  
  static AnalogData _none;

  friend class ClientBase;
};

#include "analogData.I"

#endif
