// Filename: showHideNameClass.h
// Created by:  drose (26Apr00)
// 
////////////////////////////////////////////////////////////////////

#ifndef SHOWHIDENAMECLASS_H
#define SHOWHIDENAMECLASS_H

#include <pandabase.h>

////////////////////////////////////////////////////////////////////
//       Class : ShowHideNameClass
// Description : This is a stupid little class that's used by
//               ShowHideTransition to define the name of its
//               PT(Camera) class, so the MultiTransition it
//               inherits from can initialize itself properly.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ShowHideNameClass {
public:
  static string get_class_name() {
    return "PT(Camera)";
  }
};

#endif


