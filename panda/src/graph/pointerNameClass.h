// Filename: pointerNameClass.h
// Created by:  drose (23Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef POINTERNAMECLASS_H
#define POINTERNAMECLASS_H

#include <pandabase.h>

////////////////////////////////////////////////////////////////////
//       Class : PointerNameClass
// Description : This is a stupid little class that's used by
//               MultiNodeTransition to define the name of its
//               PT(Node) class, so the MultiTransition it
//               inherits from can initialize itself properly.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PointerNameClass {
public:
  static string get_class_name() {
    return "PT(Node)";
  }
};

#endif


