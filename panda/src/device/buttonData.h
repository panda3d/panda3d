// Filename: buttonData.h
// Created by:  jason (07Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef BUTTON_DATA
#define BUTTON_DATA

#include <pandabase.h>
#include <vector_float.h>

class EXPCL_PANDA ButtonData {
public:
  INLINE ButtonData();
  INLINE ButtonData(const ButtonData &copy);
  INLINE ButtonData &operator = (const ButtonData &copy);

  INLINE static const ButtonData &none();

  double btime;
  int button_id;
  int state;
private:
  static ButtonData _none;
};

#include "buttonData.I"

#endif
