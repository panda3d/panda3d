// Filename: lightNameClass.h
// Created by:  drose (24Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef LIGHTNAMECLASS_H
#define LIGHTNAMECLASS_H

#include <pandabase.h>

////////////////////////////////////////////////////////////////////
//       Class : LightNameClass
// Description : This is a stupid little class that's used by
//               LightTransition to define the name of its
//               PT(Light) class, so the MultiTransition it
//               inherits from can initialize itself properly.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA LightNameClass {
public:
  static string get_class_name() {
    return "PT(Light)";
  }
};

#endif


